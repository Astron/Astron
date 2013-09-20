#include "ClientMessages.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

using boost::asio::ip::tcp;

ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
ConfigVariable<std::string> server_version("version", "dev");

enum ClientState
{
	CS_PRE_HELLO,
	CS_PRE_AUTH,
	CS_AUTHENTICATED
};

class Client : public NetworkClient, public MDParticipantInterface
{
	private:
		ClientState m_state;
		LogCategory *m_log;
		std::string m_client_name;
		RoleConfig m_roleconfig;
	public:
		Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig) :
			NetworkClient(socket), m_state(CS_PRE_HELLO), m_log(log), m_roleconfig(roleconfig)
		{
			std::stringstream ss;
			ss << "Client(" << socket->remote_endpoint().address().to_string() << ":"
				<< socket->remote_endpoint().port() << "): ";
			m_client_name = ss.str();
		}

		//for participant interface
		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
		{
			channel_t sender = dgi.read_uint64();
			uint16_t msgtype = dgi.read_uint16();
			switch(msgtype)
			{
			case CLIENTAGENT_DISCONNECT:
			{
				uint16_t reason = dgi.read_uint16();
				std::string error_string = dgi.read_string();
				m_log->info() << "Recv'd upstream request to DC client. Reason: "
					<< reason << " Err str: " << error_string << std::endl;
				send_disconnect(reason, error_string);
				return;
			}
			break;
			default:
				m_log->error() << "Recv'd unk server msgtype " << msgtype << std::endl;
			}
		}

	private:
		virtual void network_datagram(Datagram &dg)
		{
			switch(m_state)
			{
				case CS_PRE_HELLO:
					handle_pre_hello(dg);
					break;
				case CS_PRE_AUTH:
					handle_pre_auth(dg);
					break;
				case CS_AUTHENTICATED:
					//handle_authenticated(dg);
					break;
			}
		}

		void send_disconnect(uint16_t reason, const std::string &error_string)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_GO_GET_LOST);
			resp.add_uint16(reason);
			resp.add_string(error_string);
			network_send(resp);
			do_disconnect();
		}

		void handle_pre_hello(Datagram &dg)
		{
			DatagramIterator dgi(dg);
			uint16_t msg_type = dgi.read_uint16();
			if(msg_type != CLIENT_HELLO)
			{
				m_log->error() << m_client_name << "SECURITY WARNING: Client sent packet other than "
					"CLIENT_HELLO, pre-hello" << std::endl;
				send_disconnect(CLIENT_DISCONNECT_NO_HELLO, "");
				return;
			}

			uint32_t dc_hash = dgi.read_uint32();
			if(dc_hash != gDCF->get_hash())
			{
				m_log->error() << m_client_name << "Wrong DC hash. Got: "
					<< std::hex << dc_hash << " expected " <<
					gDCF->get_hash() << std::dec << std::endl;
				send_disconnect(CLIENT_DISCONNECT_BAD_DCHASH, "");
				return;
			}
			std::string version = dgi.read_string();

			if(version != server_version.get_rval(m_roleconfig))
			{
				m_log->error() << m_client_name << "Wrong Server version. Got: "
					<< version << " expected: " << server_version.get_rval(m_roleconfig);
				send_disconnect(CLIENT_DISCONNECT_BAD_VERSION, server_version.get_rval(m_roleconfig));
				return;
			}

			Datagram resp;
			resp.add_uint16(CLIENT_HELLO_RESP);
			network_send(resp);

			m_state = CS_PRE_AUTH;
		}

		void handle_pre_auth(Datagram &dg)
		{
			DatagramIterator dgi(dg);
			uint16_t msg_type = dgi.read_uint16();
			switch(msg_type)
			{
			case CLIENT_OBJECT_UPDATE_FIELD:
			{
				uint32_t do_id = dgi.read_uint32();
				uint16_t field_id = dgi.read_uint16();
				DCClass *dcc = NULL;
				YAML::Node uberdogs = gConfig->copy_node()["uberdogs"];
				for(auto it = uberdogs.begin(); it != uberdogs.end(); ++it)
				{
					YAML::Node uberdog = *it;
					if(uberdog["id"].as<uint32_t>() == do_id)
					{
						if(!uberdog["anonymous"].as<bool>())
						{
							send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION, "");
							return;
						}
						dcc = gDCF->get_class_by_name(uberdog["class"].as<std::string>());
					}
				}
				if(!dcc)
				{
					send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION, "");
					return;
				}

				DCField *field = dcc->get_field_by_index(field_id);
				if(!field->is_clsend())
				{
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, "field does not have clsend");
					return;
				}

				Datagram resp;
				resp.add_server_header(do_id, 0, STATESERVER_OBJECT_UPDATE_FIELD);
				dgi.seek(2);
				resp.add_data(dgi.read_remainder());
				send(resp);
			}
			break;
			default:
				m_log->warning() << m_client_name << "Recv'd unk msg type " << msg_type << std::endl;
			}
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
			new Client(socket, m_log, m_roleconfig); //It deletes itsself when connection is lost
			start_accept();
		}

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{

		}
};

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
