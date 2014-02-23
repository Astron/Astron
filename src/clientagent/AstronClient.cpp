#include "Client.h"
#include "ClientMessages.h"
#include "ClientFactory.h"
#include "ClientAgent.h"
#include "util/NetworkClient.h"
#include "core/global.h"
#include "core/msgtypes.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"

using dclass::Class;
using dclass::Field;

static ConfigVariable<bool> relocate_owned("relocate", false, ca_client_config);

class AstronClient : public Client, public NetworkClient
{
	private:
		bool m_clean_disconnect;
		bool m_relocate_owned;

	public:
		AstronClient(ConfigNode config, ClientAgent* client_agent,
			         boost::asio::ip::tcp::socket *socket) :
			Client(client_agent), NetworkClient(socket),
			m_clean_disconnect(false), m_relocate_owned(relocate_owned.get_rval(config))
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

			// Create event for EventLogger
			std::list<std::string> event;
			event.push_back("client-connected");

			// Add remote endpoint to log
			ss.str(""); // empty the stream
			ss << remote.address().to_string()
			   << ":" << remote.port();
			event.push_back(ss.str());

			// Add local endpoint to log
			ss.str(""); // empty the stream
			ss << socket->local_endpoint().address().to_string()
			   << ":" << socket->local_endpoint().port();
			event.push_back(ss.str());

			// Log created event
			log_event(event);
		}

		// send_disconnect must close any connections with a connected client; the given reason and
		// error should be forwarded to the client. Additionaly, it is recommend to log the event.
		// Handler for CLIENTAGENT_EJECT.
		void send_disconnect(uint16_t reason, const std::string &error_string, bool security = false)
		{
			if(is_connected())
			{
				Client::send_disconnect(reason, error_string, security);

				Datagram resp;
				resp.add_uint16(CLIENT_EJECT);
				resp.add_uint16(reason);
				resp.add_string(error_string);
				send_datagram(resp);

				m_clean_disconnect = true;
				NetworkClient::send_disconnect();
			}
		}

		// receive_datagram is the handler for datagrams received over the network from a Client.
		void receive_datagram(Datagram &dg)
		{
			DatagramIterator dgi(dg);
			try
			{
				switch(m_state)
				{
					// Client has just connected and should only send "CLIENT_HELLO".
					case CLIENT_STATE_NEW:
						handle_pre_hello(dgi);
						break;
					// Client has sent "CLIENT_HELLO" and can now access anonymous uberdogs.
					case CLIENT_STATE_ANONYMOUS:
						handle_pre_auth(dgi);
						break;
					// An Uberdog or AI has declared the Client authenticated and the client
					// can now interact with the server cluster normally.
					case CLIENT_STATE_ESTABLISHED:
						handle_authenticated(dgi);
						break;
				}
			}
			catch(DatagramIteratorEOF &e)
			{
				// Occurs when a handler attempts to read past end of datagram
				send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM,
				                "Datagram unexpectedly ended while iterating.");
				return;
			}
			catch(DatagramOverflow &e)
			{
				// Occurs when a handler attempts to prepare or forward a datagram to be sent
				// internally and, the resulting datagram is larger than the max datagram size.
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM,
				                "ClientDatagram too large to be routed on MD.", true);
				return;
			}

			if(dgi.get_remaining())
			{
				// All client handlers should consume all data in the datagram (for validation and security).
				// If the handler read all the data it expected, and some remains, the datagram was sent with
				// additional junk data on the end.
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Datagram contains excess data.", true);
				return;
			}
		}

		// receive_disconnect is called when the Client closes the tcp
		//     connection or otherwise when the tcp connection is lost.
		// Note: In the Astron client protocol, the server is normally
		//       responsible for terminating the connection.
		void receive_disconnect()
		{
			if(!m_clean_disconnect)
			{
				std::list<std::string> event;
				event.push_back("client-lost");
				log_event(event);
			}
			delete this;
		}

		// forward_datagram should foward the datagram to the client, or where appopriate parse
		// the packet and send the appropriate equivalent data.
		// Handler for CLIENTAGENT_SEND_DATAGRAM.
		void forward_datagram(Datagram &dg)
		{
			send_datagram(dg);
		}

		// handle_drop should immediately disconnect the client without sending any more data.
		// Handler for CLIENTAGENT_DROP.
		void handle_drop()
		{
			m_clean_disconnect = true;
			NetworkClient::send_disconnect();
		}

		// handle_add_interest should inform the client of an interest added by the server.
		void handle_add_interest(const Interest& i, uint32_t context)
		{
			bool multiple = i.zones.size() > 1;

			Datagram resp;
			resp.add_uint16(multiple ? CLIENT_ADD_INTEREST_MULTIPLE : CLIENT_ADD_INTEREST);
			resp.add_uint32(context);
			resp.add_uint16(i.id);
			resp.add_doid(i.parent);
			if(multiple)
			{
				resp.add_uint16(i.zones.size());
			}
			for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
			{
				resp.add_zone(*it);
			}
			send_datagram(resp);
		}

		// handle_remove_interest should inform the client an interest was removed by the server.
		void handle_remove_interest(uint16_t interest_id, uint32_t context)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_REMOVE_INTEREST);
			resp.add_uint32(context);
			resp.add_uint16(interest_id);
			send_datagram(resp);
		}

		// handle_add_object should inform the client of a new object. The datagram iterator
		// provided starts at the 'required fields' data, and may have optional fields following.
		// Handler for OBJECT_ENTER_LOCATION (an object, enters the Client's interest).
		void handle_add_object(doid_t do_id, doid_t parent_id, zone_t zone_id, uint16_t dc_id,
		                       DatagramIterator &dgi, bool other)
		{
			Datagram resp;
			resp.add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER : CLIENT_ENTER_OBJECT_REQUIRED);
			resp.add_doid(do_id);
			resp.add_location(parent_id, zone_id);
			resp.add_uint16(dc_id);
			resp.add_data(dgi.read_remainder());
			send_datagram(resp);
		}

		// handle_add_ownership should inform the client it has control of a new object. The datagram
		// iterator provided starts at the 'required fields' data, and may have 'optional fields'.
		// Handler for OBJECT_ENTER_OWNER (an object, enters the Client's ownership).
		void handle_add_ownership(doid_t do_id, doid_t parent_id, zone_t zone_id, uint16_t dc_id,
		                          DatagramIterator &dgi, bool other)
		{
			Datagram resp;
			resp.add_uint16(other ? CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER
			                : CLIENT_ENTER_OBJECT_REQUIRED_OWNER);
			resp.add_doid(do_id);
			resp.add_location(parent_id, zone_id);
			resp.add_uint16(dc_id);
			resp.add_data(dgi.read_remainder());
			send_datagram(resp);
		}

		// handle_set_field should inform the client that the field has been updated.
		void handle_set_field(doid_t do_id, uint16_t field_id, DatagramIterator &dgi)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_SET_FIELD);
			resp.add_doid(do_id);
			resp.add_uint16(field_id);
			resp.add_data(dgi.read_remainder());
			send_datagram(resp);
		}

		// handle_change_location should inform the client that the objects location has changed.
		void handle_change_location(doid_t do_id, doid_t new_parent, zone_t new_zone)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LOCATION);
			resp.add_doid(do_id);
			resp.add_location(new_parent, new_zone);
			send_datagram(resp);
		}

		// handle_remove_object should send a mesage to remove the object from the connected client.
		// Handler for cases where an object is no longer visible to the client;
		//     for example, when it changes zone, leaves visibility, or is deleted.
		void handle_remove_object(doid_t do_id)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING);
			resp.add_doid(do_id);
			send_datagram(resp);
		}

		// handle_remove_ownership should notify the client it no has control of the object.
		// Handle when the client loses ownership of an object.
		void handle_remove_ownership(doid_t do_id)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING_OWNER);
			resp.add_doid(do_id);
			send_datagram(resp);
		}

		// handle_interest_done is called when all of the objects from an opened interest have been
		// received. Typically, informs the client that a particular group of objects is loaded.
		void handle_interest_done(uint16_t interest_id, uint32_t context)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
			resp.add_uint32(context);
			resp.add_uint16(interest_id);
			send_datagram(resp);
		}

		// Client has just connected and should only send "CLIENT_HELLO"
		// Only handles one message type, so it does not need to be split up.
		void handle_pre_hello(DatagramIterator &dgi)
		{
			uint16_t msg_type = dgi.read_uint16();
			if(msg_type != CLIENT_HELLO)
			{
				send_disconnect(CLIENT_DISCONNECT_NO_HELLO, "First packet is not CLIENT_HELLO");
				return;
			}

			uint32_t dc_hash = dgi.read_uint32();
			std::string version = dgi.read_string();

			if(version != m_client_agent->get_version())
			{
				std::stringstream ss;
				ss << "Client version mismatch: server=" << m_client_agent->get_version() << ", client=" << version;
				send_disconnect(CLIENT_DISCONNECT_BAD_VERSION, ss.str());
				return;
			}

			const static uint32_t expected_hash = m_client_agent->get_hash();
			if(dc_hash != expected_hash)
			{
				std::stringstream ss;
				ss << "Client DC hash mismatch: server=0x" << std::hex << expected_hash << ", client=0x" << dc_hash;
				send_disconnect(CLIENT_DISCONNECT_BAD_DCHASH, ss.str());
				return;
			}

			Datagram resp;
			resp.add_uint16(CLIENT_HELLO_RESP);
			send_datagram(resp);

			m_state = CLIENT_STATE_ANONYMOUS;
		}

		// Client has sent "CLIENT_HELLO" and can now access anonymous uberdogs.
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
					NetworkClient::send_disconnect();
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

		// An Uberdog or AI has declared the Client authenticated and the client
		// can now interact with the server cluster normally.
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
					NetworkClient::send_disconnect();
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

		// handle_client_object_update_field occurs when a client sends an OBJECT_SET_FIELD
		void handle_client_object_update_field(DatagramIterator &dgi)
		{
			doid_t do_id = dgi.read_doid();
			uint16_t field_id = dgi.read_uint16();

			// Get class of object from cache
			const Class *dcc = lookup_object(do_id);

			// If the class couldn't be found, error out:
			if(!dcc)
			{
				if(is_historical_object(do_id))
				{
					// The client isn't disconnected in this case because it could be a delayed
					// message, we also have to skip to the end so a disconnect overside_datagram
					// is not sent.
					// TODO: Allow configuration to limit how long historical objects remain,
					//       for example with a timeout or bad-message limit.
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

			// Check that the client sent a field that actually exists in the class.
			const Field *field = dcc->get_field_by_id(field_id);
			if(!field)
			{
				std::stringstream ss;
				ss << "Client tried to send update for nonexistent field " << field_id << " to object "
				   << dcc->get_name() << "(" << do_id << ")";
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
				return;
			}

			// Check that the client is actually allowed to send updates to this field
			bool is_owned = m_owned_objects.find(do_id) != m_owned_objects.end();
			if(!field->has_keyword("clsend") && !(is_owned && field->has_keyword("ownsend")))
			{
				std::stringstream ss;
				ss << "Client tried to send update for non-sendable field: "
				   << dcc->get_name() << "(" << do_id << ")." << field->get_name();
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
				return;
			}

			// If an exception occurs while unpacking data it will be handled by
			// receive_datagram and the client will be dc'd with "truncated datagram".
			std::vector<uint8_t> data;
			dgi.unpack_field(field, data);

			// If an exception occurs while packing data it will be handled by
			// receive_datagram and the client will be dc'd with "oversized datagram".
			Datagram resp;
			resp.add_server_header(do_id, m_channel, STATESERVER_OBJECT_SET_FIELD);
			resp.add_doid(do_id);
			resp.add_uint16(field_id);
			resp.add_data(data);
			route_datagram(resp);
		}

		// handle_client_object_location occurs when a client sends an OBJECT_LOCATION message.
		// When sent by the client, this represents a request to change the object's location.
		void handle_client_object_location(DatagramIterator &dgi)
		{
			// Check the client is configured to allow client-relocates
			if(!m_relocate_owned)
			{
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE,
				                "Owned object relocation is disabled by server.", true);
				return;
			}
			// Check that the object the client is trying manipulate actually exists
			doid_t do_id = dgi.read_doid();
			if(m_visible_objects.find(do_id) == m_visible_objects.end())
			{
				if(is_historical_object(do_id))
				{
					// The client isn't disconnected in this case because it could be a delayed
					// message, we also have to skip to the end so a disconnect overside_datagram
					// is not sent.
					// TODO: Allow configuration to limit how long historical objects remain,
					//       for example with a timeout or bad-message limit.
					dgi.skip(dgi.get_remaining());
				}
				else
				{
					std::stringstream ss;
					ss << "Client tried to manipulate unknown object " << do_id;
					send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
				}
				return;
			}

			// Check that the client is actually allowed to change the object's location
			bool is_owned = m_owned_objects.find(do_id) != m_owned_objects.end();
			if(!is_owned)
			{
				send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE,
				                "Can't relocate an object the client doesn't own", true);
				return;
			}

			// Update the object's location
			Datagram dg(do_id, m_channel, STATESERVER_OBJECT_SET_LOCATION);
			dg.add_doid(dgi.read_doid()); // Parent
			dg.add_zone(dgi.read_zone()); // Zone
			route_datagram(dg);
		}

		// handle_client_add_interest occurs is called when the client adds an interest.
		void handle_client_add_interest(DatagramIterator &dgi, bool multiple)
		{
			uint32_t context = dgi.read_uint32();

			Interest i;
			build_interest(dgi, multiple, i);
			add_interest(i, context);
		}

		// handle_client_remove_interest is called when the client removes an interest.
		void handle_client_remove_interest(DatagramIterator &dgi)
		{
			uint32_t context = dgi.read_uint32();
			uint16_t id = dgi.read_uint16();

			// check the interest actually exists to be removed
			if(m_interests.find(id) == m_interests.end())
			{
				send_disconnect(CLIENT_DISCONNECT_GENERIC, "Tried to remove a non-existing intrest", true);
				return;
			}

			Interest &i = m_interests[id];
			remove_interest(i, context);
		}
};

static ClientType<AstronClient> astron_client_fact("libastron");
