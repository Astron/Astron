#include "Client.h"
#include "ClientMessages.h"
#include "ClientFactory.h"
#include "ClientAgent.h"
#include "util/NetworkClient.h"
#include "core/global.h"

class AstronClient : public Client, public NetworkClient
{
	private:
		bool m_clean_disconnect;

	public:
		AstronClient(ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket) :
			Client(client_agent), NetworkClient(socket), m_clean_disconnect(false)
		{
			std::stringstream ss;
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
				return;
			}
			ss << "Client (" << remote.address().to_string()
			   << ":" << remote.port() << ", " << m_channel << ")";
			m_log->set_name(ss.str());
			set_con_name(ss.str());

			std::list<std::string> event;
			event.push_back("client-connected");
			ss.str("");
			ss << remote.address().to_string()
			   << ":" << remote.port();
			event.push_back(ss.str());
			ss.str("");
			ss << socket->local_endpoint().address().to_string()
			   << ":" << socket->local_endpoint().port();
			event.push_back(ss.str());
			log_event(event);
		}

		// network_datagram is the handler for datagrams received over the network from a Client.
		void network_datagram(Datagram &dg)
		{
			DatagramIterator dgi(dg);
			try
			{
				switch(m_state)
				{
					case CLIENT_STATE_NEW:
						handle_pre_hello(dgi);
						break;
					case CLIENT_STATE_ANONYMOUS:
						handle_pre_auth(dgi);
						break;
					case CLIENT_STATE_ESTABLISHED:
						handle_authenticated(dgi);
						break;
				}
			}
			catch(DatagramIteratorEOF &e)
			{
				send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM,
				                "Datagram unexpectedly ended while iterating.");
				return;
			}

			if(dgi.get_remaining())
			{
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Datagram contains excess data.", true);
				return;
			}
		}

		void send_datagram(Datagram &dg)
		{
			network_send(dg);
		}

		// send_disconnect
		void send_disconnect(uint16_t reason, const std::string &error_string, bool security = false)
		{
			if(is_connected())
			{
				(security ? m_log->security() : m_log->error())
				        << "Terminating client connection (" << reason << "): "
				        << error_string << std::endl;

				std::list<std::string> event;
				event.push_back(security ? "client-ejected-security" : "client-ejected");
				event.push_back(std::to_string(reason));
				event.push_back(error_string);
				log_event(event);

				Datagram resp;
				resp.add_uint16(CLIENT_EJECT);
				resp.add_uint16(reason);
				resp.add_string(error_string);
				network_send(resp);
				m_clean_disconnect = true;
				do_disconnect();
			}
		}

		void handle_drop()
		{
			m_clean_disconnect = true;
			do_disconnect();
		}

		void handle_add_object(uint32_t do_id, uint32_t parent_id, uint32_t zone_id, uint16_t dc_id,
		                       DatagramIterator &dgi, bool other)
		{
			Datagram resp;
			resp.add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER : CLIENT_ENTER_OBJECT_REQUIRED);
			resp.add_uint32(do_id);
			resp.add_uint32(parent_id);
			resp.add_uint32(zone_id);
			resp.add_uint16(dc_id);
			resp.add_data(dgi.read_remainder());
			network_send(resp);
		}

		void handle_add_ownership(uint32_t do_id, uint32_t parent_id, uint32_t zone_id, uint16_t dc_id,
		                          DatagramIterator &dgi, bool other)
		{
			Datagram resp;
			resp.add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER
			                : CLIENT_ENTER_OBJECT_REQUIRED_OWNER);
			resp.add_uint32(do_id);
			resp.add_uint32(parent_id);
			resp.add_uint32(zone_id);
			resp.add_uint16(dc_id);
			resp.add_data(dgi.read_remainder());
			network_send(resp);
		}

		void handle_set_field(uint32_t do_id, uint16_t field_id, const std::vector<uint8_t> &value)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_SET_FIELD);
			resp.add_uint32(do_id);
			resp.add_uint16(field_id);
			resp.add_data(value);
			network_send(resp);
		}

		void handle_change_location(uint32_t do_id, uint32_t new_parent, uint32_t new_zone)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LOCATION);
			resp.add_uint32(do_id);
			resp.add_uint32(new_parent);
			resp.add_uint32(new_zone);
			network_send(resp);
		}

		void handle_remove_object(uint32_t do_id)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING);
			resp.add_uint32(do_id);
			network_send(resp);
		}

		void handle_remove_ownership(uint32_t do_id)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING_OWNER);
			resp.add_uint32(do_id);
			network_send(resp);
		}

		void handle_interest_done(uint16_t interest_id, uint32_t context)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
			resp.add_uint32(context);
			resp.add_uint16(interest_id);
			network_send(resp);
		}

		//Only handles one message type, so it does not need to be split up.
		void handle_pre_hello(DatagramIterator &dgi)
		{
			uint16_t msg_type = dgi.read_uint16();
			if(msg_type != CLIENT_HELLO)
			{
				send_disconnect(CLIENT_DISCONNECT_NO_HELLO, "First packet is not CLIENT_HELLO");
				return;
			}

			uint32_t dc_hash = dgi.read_uint32();
			const static uint32_t expected_hash = g_dcf->get_hash();
			if(dc_hash != expected_hash)
			{
				std::stringstream ss;
				ss << "Client DC hash mismatch: server=0x" << std::hex << expected_hash << ", client=0x" << dc_hash;
				send_disconnect(CLIENT_DISCONNECT_BAD_DCHASH, ss.str());
				return;
			}

			std::string version = dgi.read_string();
			if(version != m_client_agent->get_version())
			{
				std::stringstream ss;
				ss << "Client version mismatch: server=" << m_client_agent->get_version() << ", client=" << version;
				send_disconnect(CLIENT_DISCONNECT_BAD_VERSION, ss.str());
				return;
			}

			Datagram resp;
			resp.add_uint16(CLIENT_HELLO_RESP);
			network_send(resp);

			m_state = CLIENT_STATE_ANONYMOUS;
		}

		void handle_pre_auth(DatagramIterator &dgi)
		{
			uint16_t msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case CLIENT_DISCONNECT:
				{
					std::list<std::string> event;
					event.push_back("client-disconnected");
					log_event(event);

					m_clean_disconnect = true;
					do_disconnect();
				}
				break;
				case CLIENT_OBJECT_SET_FIELD:
				{
					handle_client_object_update_field(dgi);
				}
				break;
				default:
					std::stringstream ss;
					ss << "Message type " << msg_type << " not allowed prior to authentication.";
					send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
					return;
			}
		}

		void handle_authenticated(DatagramIterator &dgi)
		{
			uint16_t msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case CLIENT_DISCONNECT:
				{
					std::list<std::string> event;
					event.push_back("client-disconnected");
					log_event(event);

					m_clean_disconnect = true;
					do_disconnect();
				}
				break;
				case CLIENT_OBJECT_SET_FIELD:
					handle_client_object_update_field(dgi);
					break;
				case CLIENT_OBJECT_LOCATION:
					handle_client_object_location(dgi);
					break;
				case CLIENT_ADD_INTEREST:
					handle_client_add_interest(dgi, false);
					break;
				case CLIENT_ADD_INTEREST_MULTIPLE:
					handle_client_add_interest(dgi, true);
					break;
				case CLIENT_REMOVE_INTEREST:
					handle_client_remove_interest(dgi);
					break;
				default:
					std::stringstream ss;
					ss << "Message type " << msg_type << " not valid.";
					send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
					return;
			}
		}

		void handle_client_object_update_field(DatagramIterator &dgi)
		{
			uint32_t do_id = dgi.read_uint32();
			uint16_t field_id = dgi.read_uint16();

			DCClass *dcc = lookup_object(do_id);

			// If the class couldn't be found, error out:
			if(!dcc)
			{
				if(is_historical_object(do_id))
				{
					dgi.skip(dgi.get_remaining());
				}
				else
				{
					std::stringstream ss;
					ss << "Client tried to send update to nonexistent object " << do_id;
					send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
				}
				return;
			}

			// If the client is not in the ESTABLISHED state, it may only send updates
			// to anonymous UberDOGs.
			if(m_state != CLIENT_STATE_ESTABLISHED)
			{
				if(g_uberdogs.find(do_id) == g_uberdogs.end() || !g_uberdogs[do_id].anonymous)
				{
					std::stringstream ss;
					ss << "Client tried to send update to non-anonymous object "
					   << dcc->get_name() << "(" << do_id << ")";
					send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION, ss.str(), true);
					return;
				}
			}


			DCField *field = dcc->get_field_by_index(field_id);
			if(!field)
			{
				std::stringstream ss;
				ss << "Client tried to send update for nonexistent field " << field_id << " to object "
				   << dcc->get_name() << "(" << do_id << ")";
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
				return;
			}

			bool is_owned = m_owned_objects.find(do_id) != m_owned_objects.end();

			if(!field->is_clsend() && !(is_owned && field->is_ownsend()))
			{
				std::stringstream ss;
				ss << "Client tried to send update for non-sendable field: "
				   << dcc->get_name() << "(" << do_id << ")." << field->get_name();
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
				return;
			}

			std::vector<uint8_t> data;
			dgi.unpack_field(field, data);//if an exception occurs it will be handled
			//and client will be dc'd for truncated datagram

			Datagram resp;
			resp.add_server_header(do_id, m_channel, STATESERVER_OBJECT_SET_FIELD);
			resp.add_uint32(do_id);
			resp.add_uint16(field_id);
			if(data.size() > 65535u - resp.size())
			{
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Field update too large to be routed on MD.",
				                true);
				return;
			}
			resp.add_data(data);
			send(resp);
		}

		void handle_client_object_location(DatagramIterator &dgi)
		{
			uint32_t do_id = dgi.read_uint32();
			if(m_dist_objs.find(do_id) == m_dist_objs.end())
			{
				std::stringstream ss;
				ss << "Client tried to manipulate unknown object " << do_id;
				send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
				return;
			}
			bool is_owned = false;
			for(auto it = m_owned_objects.begin(); it != m_owned_objects.end(); ++it)
			{
				if(*it == do_id)
				{
					is_owned = true;
					break;
				}
			}

			if(!is_owned)
			{
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE,
				                "Can't relocate an object the client doesn't own", true);
				return;
			}

			Datagram dg(do_id, m_channel, STATESERVER_OBJECT_SET_LOCATION);
			dg.add_uint32(dgi.read_uint32()); // Parent
			dg.add_uint32(dgi.read_uint32()); // Zone
			send(dg);
		}

		void handle_client_add_interest(DatagramIterator &dgi, bool multiple)
		{
			uint32_t context = dgi.read_uint32();
			uint16_t interest_id = dgi.read_uint16();
			uint32_t parent = dgi.read_uint32();

			Interest i;
			i.id = interest_id;
			i.parent = parent;

			uint16_t count = 1;
			if(multiple)
			{
				count = dgi.read_uint16();
			}
			i.zones.reserve(count);
			for(int x = 0; x < count; ++x)
			{
				uint32_t zone = dgi.read_uint32();
				i.zones.insert(i.zones.end(), zone);
			}

			add_interest(i, context);
		}

		void handle_client_remove_interest(DatagramIterator &dgi)
		{
			uint32_t context = dgi.read_uint32();
			uint16_t id = dgi.read_uint16();
			if(m_interests.find(id) == m_interests.end())
			{
				send_disconnect(CLIENT_DISCONNECT_GENERIC, "Tried to remove a non-existing intrest", true);
				return;
			}
			Interest &i = m_interests[id];
			remove_interest(i, context);
		}

		void network_disconnect()
		{
			if(!m_clean_disconnect)
			{
				std::list<std::string> event;
				event.push_back("client-lost");
				log_event(event);
			}
			delete this;
		}

};

static ClientType<AstronClient> astron_client_fact("libastron");
