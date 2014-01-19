#include "ClientAgent.h"
#include "ClientFactory.h"

#include <boost/bind.hpp>

#include "core/global.h"
#include "core/RoleFactory.h"

using boost::asio::ip::tcp;

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
static ConfigVariable<std::string> client_type("client", "libastron");
static ConfigVariable<std::string> server_version("version", "dev");
static ConfigVariable<uint32_t> override_hash("manual_dc_hash", 0x0);
static ConfigVariable<channel_t> min_channel("channels/min", INVALID_CHANNEL);
static ConfigVariable<channel_t> max_channel("channels/max", INVALID_CHANNEL);

ClientAgent::ClientAgent(RoleConfig roleconfig) : Role(roleconfig), m_acceptor(NULL),
	m_client_type(client_type.get_rval(roleconfig)),
	m_server_version(server_version.get_rval(roleconfig)),
	m_ct(min_channel.get_rval(roleconfig), max_channel.get_rval(roleconfig))
{
	std::stringstream ss;
	ss << "Client Agent (" << bind_addr.get_rval(roleconfig) << ")";
	m_log = new LogCategory("clientagent", ss.str());

	// Get DC Hash
	const uint32_t config_hash = override_hash.get_rval(roleconfig);
	if(config_hash > 0x0)
	{
		m_hash = config_hash;
	}
	else
	{
		m_hash = g_dcf->get_hash();		
	}

	//Initialize the network
	std::string str_ip = bind_addr.get_rval(m_roleconfig);
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
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
	catch (std::exception &e)
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
	               << remote.address() << ":" << remote.port() << std::endl;
	ClientFactory::singleton.instantiate_client(m_client_type, this, socket);
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