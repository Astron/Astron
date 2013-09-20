#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

using boost::asio::ip::tcp;

ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
ConfigVariable<std::string> server_version("server_version", "dev");

enum ClientState
{
	CS_PRE_HELLO,
	CS_PRE_AUTH,
	CS_AUTHED
};

class Client : public NetworkClient
{
	private:
		ClientState m_state;
	public:
		Client(boost::asio::ip::tcp::socket *socket) : NetworkClient(socket),
			m_state(CS_PRE_HELLO)
		{
		}

	private:
		virtual void network_datagram(Datagram &dg)
		{
		}

		virtual void network_disconnect()
		{
			delete this;
		}
};

class ClientAgent : public Role
{
	private:
		LogCategory *m_log;
		tcp::acceptor *m_acceptor;

	public:
		ClientAgent(RoleConfig roleconfig) : Role(roleconfig),
			m_acceptor(NULL)
		{
			std::stringstream ss;
			ss << "Client Agent (" << bind_addr.get_rval(roleconfig) << ")";
			m_log = new LogCategory("clientagent", ss.str());

			//Initialize the network
			std::string str_ip = bind_addr.get_rval(m_roleconfig);
			std::string str_port = str_ip.substr(str_ip.find(':', 0)+1, std::string::npos);
			str_ip = str_ip.substr(0, str_ip.find(':', 0));
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(str_ip, str_port);
			tcp::resolver::iterator it = resolver.resolve(query);
			m_acceptor = new tcp::acceptor(io_service, *it, true);
			start_accept();
		}

		~ClientAgent()
		{
			delete m_log;
		}

		void start_accept()
		{
			tcp::socket *socket = new tcp::socket(io_service);
			tcp::endpoint peerEndpoint;
			m_acceptor->async_accept(*socket, boost::bind(&ClientAgent::handle_accept, 
				this, socket, boost::asio::placeholders::error));
		}

		void handle_accept(tcp::socket *socket, const boost::system::error_code &ec)
		{
			boost::asio::ip::tcp::endpoint remote = socket->remote_endpoint();
			m_log->info() << "Got an incoming connection from "
						 << remote.address() << ":" << remote.port() << std::endl;
			new Client(socket); //It deletes itsself when connection is lost
			start_accept();
		}

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{

		}
};

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
