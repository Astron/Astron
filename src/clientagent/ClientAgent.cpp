#include "ClientAgent.h"
#include "ClientFactory.h"

#include <boost/filesystem.hpp>
#include "core/global.h"
#include "core/shutdown.h"
#include "core/RoleFactory.h"
#include "config/constraints.h"
#include "dclass/file/hash.h"
#include "net/TcpAcceptor.h"
#include "net/SslAcceptor.h"
#include "util/password_prompt.h"
using namespace std;
namespace ssl = boost::asio::ssl;
namespace filesystem = boost::filesystem;

RoleConfigGroup clientagent_config("clientagent");
static ConfigVariable<string> bind_addr("bind", "0.0.0.0:7198", clientagent_config);
static ConfigVariable<string> server_version("version", "dev", clientagent_config);
static ConfigVariable<bool> behind_haproxy("haproxy", false, clientagent_config);
static ConfigVariable<uint32_t> override_hash("manual_dc_hash", 0x0, clientagent_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);

static ConfigGroup tls_config("tls", clientagent_config);
static ConfigVariable<string> tls_cert("certificate", "", tls_config);
static ConfigVariable<string> tls_key("key_file", "", tls_config);
static ConfigVariable<string> tls_chain("chain_file", "", tls_config);
static ConfigVariable<string> tls_auth("cert_authority", "", tls_config);
static ConfigVariable<unsigned int> tls_verify_depth("max_verify_depth", 6, tls_config);
static ConfigVariable<bool> sslv2_enabled("sslv2", false, tls_config);
static ConfigVariable<bool> sslv3_enabled("sslv3", false, tls_config);
static ConfigVariable<bool> tlsv1_enabled("tlsv1", true, tls_config);
static ConfigVariable<int> tls_handshake_timeout("handshake_timeout", 5000, tls_config);
static FileAvailableConstraint tls_cert_exists(tls_cert);
static FileAvailableConstraint tls_key_exists(tls_key);
static FileAvailableConstraint tls_chain_exists(tls_chain);
static FileAvailableConstraint tls_auth_exists(tls_auth);
static BooleanValueConstraint sslv2_is_boolean(sslv2_enabled);
static BooleanValueConstraint sslv3_is_boolean(sslv3_enabled);
static BooleanValueConstraint tlsv1_is_boolean(tlsv1_enabled);

static ConfigGroup channels_config("channels", clientagent_config);
static ConfigVariable<channel_t> min_channel("min", INVALID_CHANNEL, channels_config);
static ConfigVariable<channel_t> max_channel("max", INVALID_CHANNEL, channels_config);
static InvalidChannelConstraint min_not_invalid(min_channel);
static InvalidChannelConstraint max_not_invalid(max_channel);
static ReservedChannelConstraint min_not_reserved(min_channel);
static ReservedChannelConstraint max_not_reserved(max_channel);

KeyedConfigGroup ca_client_config("client", "type", clientagent_config, "libastron");
ConfigVariable<string> ca_client_type("type", "libastron", ca_client_config);
bool have_client_type(const string& backend)
{
    return ClientFactory::singleton().has_client_type(backend);
}
ConfigConstraint<string> client_type_exists(have_client_type, ca_client_type,
        "No Client handler exists for the given client type.");

static ConfigGroup tuning_config("tuning", clientagent_config);
static ConfigVariable<unsigned long> interest_timeout("interest_timeout", 500, tuning_config);

ClientAgent::ClientAgent(RoleConfig roleconfig) : Role(roleconfig), m_net_acceptor(nullptr),
    m_server_version(server_version.get_rval(roleconfig)), m_ssl_ctx(ssl::context::sslv23)
{

    stringstream ss;
    ss << "Client Agent (" << bind_addr.get_rval(roleconfig) << ")";
    m_log = std::unique_ptr<LogCategory>(new LogCategory("clientagent", ss.str()));

    // We need to get the client type...
    ConfigNode client = clientagent_config.get_child_node(ca_client_config, roleconfig);
    m_client_type = ca_client_type.get_rval(client);

    // ... and also the channel range ...
    ConfigNode channels = clientagent_config.get_child_node(channels_config, roleconfig);
    m_ct = ChannelTracker(min_channel.get_rval(channels), max_channel.get_rval(channels));

    // ... then store a copy of the client config.
    m_clientconfig = clientagent_config.get_child_node(ca_client_config, roleconfig);

    // Calculate the DC hash
    const uint32_t config_hash = override_hash.get_rval(roleconfig);
    if(config_hash > 0x0) {
        m_hash = config_hash;
    } else {
        m_hash = dclass::legacy_hash(g_dcf);
    }

    // Load tuning parameters.
    ConfigNode tuning = clientagent_config.get_child_node(tuning_config, roleconfig);
    m_interest_timeout = interest_timeout.get_rval(tuning);

    // Load SSL data from Config vars
    ConfigNode tls_settings = clientagent_config.get_child_node(tls_config, roleconfig);

    m_ssl_cert = tls_cert.get_rval(tls_settings);
    m_ssl_key = tls_key.get_rval(tls_settings);

    // Handle no SSL
    if(m_ssl_cert.empty() && m_ssl_key.empty()) {
        m_log->debug() << "Not using SSL/TLS.\n";
        TcpAcceptorCallback callback = std::bind(&ClientAgent::handle_tcp, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3);
        m_net_acceptor = std::unique_ptr<TcpAcceptor>(new TcpAcceptor(io_service, callback));
    }

    // Handle SSL requested, but some information missing
    else if(m_ssl_cert.empty() != m_ssl_key.empty()) {
        m_log->fatal() << "TLS requested but either certificate or key is missing.\n";
        astron_shutdown(1);
    }

    // Handle SSL
    else {
        m_log->debug() << "Configured with SSL/TLS.\n";

        // Get the enabled SSL/TLS protocols
        long int options = ssl::context::default_workarounds;
        if(!sslv2_enabled.get_rval(tls_settings)) {
            options |= ssl::context::no_sslv2;
        }
        if(!sslv3_enabled.get_rval(tls_settings)) {
            options |= ssl::context::no_sslv3;
        }
        if(!tlsv1_enabled.get_rval(tls_settings)) {
            options |= ssl::context::no_tlsv1;
        }

        // Create the context with enabled protocols
        m_ssl_ctx.set_options(options);

        // Set the chain file
        string chain_file = tls_chain.get_rval(tls_settings);
        if(!chain_file.empty()) {
            m_ssl_ctx.use_certificate_chain_file(chain_file);
        }

        // Set the server certificate
        m_ssl_ctx.use_certificate_file(m_ssl_cert, ssl::context::file_format::pem);

        // Set the password callback
        m_ssl_ctx.set_password_callback(boost::bind(&ClientAgent::ssl_password_callback, this));

        // Set the private key
        bool key_error = false;
        for(int attempts = 0; attempts < 3; ++attempts) {
            try {
                m_ssl_ctx.use_private_key_file(m_ssl_key, ssl::context::file_format::pem);
                key_error = false;
                break;
            } catch(const boost::system::system_error&) {
                key_error = true;
            }
        }
        if(key_error) {
            m_log->fatal() << "Could not open SSL key file (" << m_ssl_key << ").\n";
            exit(1);
        }

        // Set the certificate authority
        string auth_file = tls_auth.get_rval(tls_settings);
        if(!auth_file.empty()) {
            if(filesystem::is_directory(auth_file)) {
                m_ssl_ctx.add_verify_path(auth_file);
            } else {
                m_ssl_ctx.load_verify_file(auth_file);
            }

            m_ssl_ctx.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);
            m_ssl_ctx.set_verify_depth(tls_verify_depth.get_rval(tls_settings));
        }

        SslAcceptorCallback callback = std::bind(&ClientAgent::handle_ssl, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3);
        std::unique_ptr<SslAcceptor> ssl_acceptor(new SslAcceptor(io_service, m_ssl_ctx, callback));

        // Set SSL handshake timeout.
        ssl_acceptor->set_handshake_timeout(tls_handshake_timeout.get_rval(tls_settings));

        m_net_acceptor = std::move(ssl_acceptor);
    }

    m_net_acceptor->set_haproxy_mode(behind_haproxy.get_rval(m_roleconfig));

    // Begin listening for new Clients
    boost::system::error_code ec;
    ec = m_net_acceptor->bind(bind_addr.get_rval(m_roleconfig), 7198);
    if(ec.value() != 0) {
        m_log->fatal() << "Could not bind listening port: "
                       << bind_addr.get_val() << std::endl;
        m_log->fatal() << "Error code: " << ec.value()
                       << "(" << ec.category().message(ec.value()) << ")\n";
        astron_shutdown(1);
    }
    m_net_acceptor->start();
}

// handle_tcp generates a new Client object from a raw tcp connection.
void ClientAgent::handle_tcp(tcp::socket *socket,
                             const tcp::endpoint &remote,
                             const tcp::endpoint &local)
{
    m_log->debug() << "Got an incoming connection from "
                   << remote.address() << ":" << remote.port() << "\n";

    ClientFactory::singleton().instantiate_client(m_client_type, m_clientconfig, this, socket, remote,
            local);
}

// handle_ssl generates a new Client object from an ssl stream.
void ClientAgent::handle_ssl(ssl::stream<tcp::socket> *stream,
                             const tcp::endpoint &remote,
                             const tcp::endpoint &local)
{
    m_log->debug() << "Got an incoming connection from "
                   << remote.address() << ":" << remote.port() << "\n";

    ClientFactory::singleton().instantiate_client(m_client_type, m_clientconfig, this, stream, remote,
            local);
}


// handle_datagram handles Datagrams received from the message director.
void ClientAgent::handle_datagram(DatagramHandle, DatagramIterator&)
{
    // At the moment, the client agent doesn't actually handle any datagrams
}

string ClientAgent::ssl_password_callback()
{
    stringstream prompt;
    prompt << "Enter password for " << m_ssl_key << ": ";
    return password_prompt(prompt.str());
}

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");



/* ========================== *
 *       HELPER CLASSES       *
 * ========================== */
ChannelTracker::ChannelTracker(channel_t min, channel_t max) : m_next(min), m_max(max),
    m_unused_channels()
{
}

channel_t ChannelTracker::alloc_channel()
{
    if(m_next <= m_max) {
        return m_next++;
    } else {
        if(!m_unused_channels.empty()) {
            channel_t c = m_unused_channels.front();
            m_unused_channels.pop();
            return c;
        }
    }
    return 0;
}

void ChannelTracker::free_channel(channel_t channel)
{
    m_unused_channels.push(channel);
}
