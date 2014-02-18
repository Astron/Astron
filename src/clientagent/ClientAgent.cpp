#include "ClientAgent.h"
#include "ClientFactory.h"

#include "core/global.h"
#include "core/RoleFactory.h"
#include "config/constraints.h"
#include "dclass/file/hash.h"
#include <boost/bind.hpp>
using boost::asio::ip::tcp;
using namespace std;

RoleConfigGroup clientagent_config("clientagent");
static ConfigVariable<string> bind_addr("bind", "0.0.0.0:7198", clientagent_config);
static ConfigVariable<string> server_version("version", "dev", clientagent_config);
static ConfigVariable<uint32_t> override_hash("manual_dc_hash", 0x0, clientagent_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);

static ConfigGroup channels_config("channels", clientagent_config);
static ConfigVariable<channel_t> min_channel("min", INVALID_CHANNEL, channels_config);
static ConfigVariable<channel_t> max_channel("max", INVALID_CHANNEL, channels_config);
static InvalidChannelConstraint min_not_invalid(min_channel);
static InvalidChannelConstraint max_not_invalid(max_channel);
static ReservedChannelConstraint min_not_reserved(min_channel);
static ReservedChannelConstraint max_not_reserved(max_channel);

ConfigGroup ca_client_config("client", clientagent_config);
ConfigVariable<string> ca_client_type("type", "libastron", ca_client_config);
bool have_client_type(const string& backend)
{
	return ClientFactory::singleton().has_client_type(backend);
}
ConfigConstraint<string> client_type_exists(have_client_type, ca_client_type,
		"No Client handler exists for the given client type.");

ClientAgent::ClientAgent(RoleConfig roleconfig) : Role(roleconfig), m_acceptor(NULL),
	m_server_version(server_version.get_rval(roleconfig))
{
	stringstream ss;
	ss << "Client Agent (" << bind_addr.get_rval(roleconfig) << ")";
	m_log = new LogCategory("clientagent", ss.str());

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
	if(config_hash > 0x0)
	{
		m_hash = config_hash;
	}
	else
	{
		m_hash = dclass::legacy_hash(g_dcf);
	}

	//Initialize the network
	string str_ip = bind_addr.get_rval(m_roleconfig);
	string str_port = str_ip.substr(str_ip.find(':', 0) + 1, string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(str_ip, str_port);
	tcp::resolver::iterator it = resolver.resolve(query);
	m_acceptor = new tcp::acceptor(io_service, *it, true);

	start_accept();
}

ClientAgent::~ClientAgent()
{
	delete m_log;
}

// start_accept waits for a new client connection and calls handle_accept when received.
void ClientAgent::start_accept()
{
	tcp::socket *socket = new tcp::socket(io_service);
	tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(*socket, boost::bind(&ClientAgent::handle_accept,
	                         this, socket, boost::asio::placeholders::error));
}

// handle_accepts generates a new Client object from a connection, then calls start_accept.
void ClientAgent::handle_accept(tcp::socket *socket, const boost::system::error_code& /*ec*/)
{
	// TODO: We probably want to check the error code here

	boost::asio::ip::tcp::endpoint remote;
	try
	{
		remote = socket->remote_endpoint();
	}
	catch (exception &e)
	{
		// A client might disconnect immediately after connecting.
		// If this happens, do nothing. Resolves #122.
		// N.B. due to a Boost.Asio bug, the socket will (may?) still have
		// is_open() == true, so we just catch the exception on remote_endpoint
		// instead.
		start_accept();
		return;
	}
	m_log->debug() << "Got an incoming connection from "
	               << remote.address() << ":" << remote.port() << endl;

	ClientFactory::singleton().instantiate_client(m_client_type, m_clientconfig, this, socket);

	start_accept();
}

// handle_datagram handles Datagrams received from the message director.
void ClientAgent::handle_datagram(Datagram&, DatagramIterator&)
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
	if(m_next <= m_max)
	{
		return m_next++;
	}
	else
	{
		if(!m_unused_channels.empty())
		{
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
