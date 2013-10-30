#include "ClientAgent.h"
#include "ClientFactory.h"

#include <boost/bind.hpp>

#include "core/global.h"
#include "core/RoleFactory.h"

using boost::asio::ip::tcp;

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
static ConfigVariable<std::string> client_type("client", "libastron");
static ConfigVariable<std::string> server_version("version", "dev");
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

	//Initialize the network
	std::string str_ip = bind_addr.get_rval(m_roleconfig);
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(str_ip, str_port);
	tcp::resolver::iterator it = resolver.resolve(query);
	m_acceptor = new tcp::acceptor(io_service, *it, true);

	if(g_uberdogs.empty())
	{
		YAML::Node udnodes = g_config->copy_node()["uberdogs"];
		if(!udnodes.IsNull())
		{
			for(auto it = udnodes.begin(); it != udnodes.end(); ++it)
			{
				YAML::Node udnode = *it;
				Uberdog ud;
				ud.dcc = g_dcf->get_class_by_name(udnode["class"].as<std::string>());
				if(!ud.dcc)
				{
					m_log->fatal() << "DCClass " << udnode["class"].as<std::string>()
					               << "Does not exist!" << std::endl;
					exit(1);
				}
				ud.anonymous = udnode["anonymous"].as<bool>();
				g_uberdogs[udnode["id"].as<uint32_t>()] = ud;
			}
		}
	}

	start_accept();
}

ClientAgent::~ClientAgent()
{
	delete m_log;
}

void ClientAgent::start_accept()
{
	tcp::socket *socket = new tcp::socket(io_service);
	tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(*socket, boost::bind(&ClientAgent::handle_accept,
	                         this, socket, boost::asio::placeholders::error));
}

void ClientAgent::handle_accept(tcp::socket *socket, const boost::system::error_code &ec)
{
	boost::asio::ip::tcp::endpoint remote = socket->remote_endpoint();
	m_log->debug() << "Got an incoming connection from "
	               << remote.address() << ":" << remote.port() << std::endl;
	ClientFactory::singleton.instantiate_client(m_client_type, this, socket);
	start_accept();
}

void ClientAgent::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
}

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
