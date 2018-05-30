#include "ClientAgent.h"
#include "ClientFactory.h"

#include "core/global.h"
#include "core/shutdown.h"
#include "core/RoleFactory.h"
#include "config/constraints.h"
#include "dclass/file/hash.h"
#include "net/TcpAcceptor.h"
using namespace std;

RoleConfigGroup clientagent_config("clientagent");
static ConfigVariable<string> bind_addr("bind", "0.0.0.0:7198", clientagent_config);
static ConfigVariable<string> server_version("version", "dev", clientagent_config);
static ConfigVariable<bool> behind_haproxy("haproxy", false, clientagent_config);
static ConfigVariable<uint32_t> override_hash("manual_dc_hash", 0x0, clientagent_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);

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
    m_server_version(server_version.get_rval(roleconfig))
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

    TcpAcceptorCallback callback = std::bind(&ClientAgent::handle_tcp, this,
                                   std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3,
                                   std::placeholders::_4);
    AcceptorErrorCallback err_callback = std::bind(&ClientAgent::handle_error, this,
                                            std::placeholders::_1);

    m_net_acceptor = std::unique_ptr<TcpAcceptor>(new TcpAcceptor(callback, err_callback));

    m_net_acceptor->set_haproxy_mode(behind_haproxy.get_rval(m_roleconfig));

    // Begin listening for new Clients
    m_net_acceptor->bind(bind_addr.get_rval(m_roleconfig), 7198);
    m_net_acceptor->start();
}

// handle_tcp generates a new Client object from a raw tcp connection.
void ClientAgent::handle_tcp(const std::shared_ptr<uvw::TcpHandle> &socket,
                             const uvw::Addr &remote,
                             const uvw::Addr &local,
                             const bool haproxy_mode)
{
    m_log->debug() << "Got an incoming connection from "
                   << remote.ip << ":" << remote.port << "\n";

    ClientFactory::singleton().instantiate_client(m_client_type, m_clientconfig, this, socket, remote, local, haproxy_mode);
}

void ClientAgent::handle_error(const uvw::ErrorEvent& evt)
{
    if(evt.code() == UV_EADDRINUSE || evt.code() == UV_EADDRNOTAVAIL) {
        m_log->fatal() << "Failed to bind to address: " << evt.what() << "\n";
        exit(1);
    }
}

// handle_datagram handles Datagrams received from the message director.
void ClientAgent::handle_datagram(DatagramHandle, DatagramIterator&)
{
    // At the moment, the client agent doesn't actually handle any datagrams
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
