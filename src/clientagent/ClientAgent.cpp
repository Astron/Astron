#include "ClientMessages.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

#include <stack>
#include <set>
#include <list>

using boost::asio::ip::tcp;

ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
ConfigVariable<std::string> server_version("version", "dev");
ConfigVariable<channel_t> min_channel("channels/min", 0);
ConfigVariable<channel_t> max_channel("channels/max", 0);

enum ClientState
{
	CS_PRE_HELLO,
	CS_PRE_AUTH,
	CS_AUTHENTICATED
};

struct DistributedObject
{
	uint32_t id;
	uint32_t parent;
	uint32_t zone;
	DCClass *dcc;
	uint32_t refcount;
};

std::map<uint32_t, DistributedObject> dist_objs;

class ChannelTracker
{
	private:
		channel_t m_next;
		channel_t m_max;
		std::stack<channel_t> m_unused_channels;
	public:
		ChannelTracker(channel_t min, channel_t max) : m_next(min),
			m_max(max), m_unused_channels()
		{
		}

		channel_t alloc_channel()
		{
			channel_t ret;
			if(!m_unused_channels.empty())
			{
				ret = m_unused_channels.top();
				m_unused_channels.pop();
			}
			else
			{
				ret = m_next++;
			}
			return ret;
		}

		void free_channel(channel_t channel)
		{
			m_unused_channels.push(channel);
		}
};

struct Interest
{
	uint32_t parent;
	std::vector<std::pair<uint32_t, bool>> zones; //bool = readiness state
	uint32_t context;

	Interest() : parent(0),
		zones(0), context(0)
	{
	}

	bool is_ready()
	{
		for(auto it = zones.begin(); it != zones.end(); ++it)
		{
			if(!it->second)
			{
				return false;
			}
		}
		return true;
	}
};

struct Uberdog
{
	DCClass *dcc;
	bool anonymous;
};

std::map<uint32_t, Uberdog> uberdogs;

class Client : public NetworkClient, public MDParticipantInterface
{
	private:
		ClientState m_state;
		LogCategory *m_log;
		std::string m_client_name;
		RoleConfig m_roleconfig;
		ChannelTracker *m_ct;
		channel_t m_channel;
		channel_t m_allocated_channel;
		bool m_is_channel_allocated;
		std::set<uint32_t> m_owned_objects;
		std::map<uint16_t, Interest> m_interests;
	public:
		Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig,
			ChannelTracker *ct) : NetworkClient(socket), m_state(CS_PRE_HELLO),
				m_log(log), m_roleconfig(roleconfig), m_ct(ct), m_channel(0),
				m_is_channel_allocated(true), m_owned_objects(), m_allocated_channel(0),
				m_interests()
		{
			m_channel = m_ct->alloc_channel();
			m_allocated_channel = m_channel;
			subscribe_channel(m_channel);
			std::stringstream ss;
			ss << "Client(" << socket->remote_endpoint().address().to_string() << ":"
				<< socket->remote_endpoint().port() << "): ";
			m_client_name = ss.str();
		}

		~Client()
		{
			for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
			{
				Interest &i = it->second;
				for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
				{
					uint32_t zone = it->first;
					for(auto it = dist_objs.begin(); it != dist_objs.end(); ++it)
					{
						if(it->second.refcount)
						{
							it->second.refcount--;
						}
					}
				}
			}
			m_ct->free_channel(m_allocated_channel);
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
			case CLIENTAGENT_DROP:
			{
				do_disconnect();
				return;
			}
			break;
			case CLIENTAGENT_SET_STATE:
			{
				m_state = (ClientState)dgi.read_uint16();
			}
			break;
			case STATESERVER_OBJECT_UPDATE_FIELD:
			{
				Datagram resp;
				resp.add_uint16(CLIENT_OBJECT_UPDATE_FIELD);
				resp.add_data(dgi.read_remainder());
				network_send(resp);
			}
			break;
			case STATESERVER_OBJECT_ENTER_OWNER_RECV:
			{
				uint32_t parent = dgi.read_uint32();
				uint32_t zone = dgi.read_uint32();
				uint16_t dc_id = dgi.read_uint16();
				uint32_t do_id = dgi.read_uint32();
				m_owned_objects.insert(do_id);

				if(dist_objs.find(do_id) == dist_objs.end() || dist_objs[do_id].refcount == 0)
				{
					DistributedObject obj;
					obj.id = do_id;
					obj.parent = parent;
					obj.zone = zone;
					obj.dcc = gDCF->get_class(dc_id);
					obj.refcount = 0;
					dist_objs[do_id] = obj;
				}
				dist_objs[do_id].refcount++;

				Datagram resp;
				resp.add_uint16(CLIENT_CREATE_OBJECT_REQUIRED_OTHER_OWNER);
				resp.add_uint32(parent);
				resp.add_uint32(zone);
				resp.add_uint16(dc_id);
				resp.add_uint32(do_id);
				resp.add_data(dgi.read_remainder());
				network_send(resp);
			}
			break;
			case CLIENTAGENT_SET_SENDER_ID:
			{
				if(m_is_channel_allocated)
				{
					m_is_channel_allocated = false;
				}
				else
				{
					unsubscribe_channel(m_channel);
				}
				m_channel = dgi.read_uint64();
				subscribe_channel(m_channel);
			}
			break;
			case CLIENTAGENT_SEND_DATAGRAM:
			{
				Datagram resp;
				resp.add_data(dgi.read_string());
				network_send(resp);
			}
			break;
			case CLIENTAGENT_OPEN_CHANNEL:
			{
				subscribe_channel(dgi.read_uint64());
			}
			break;
			case CLIENTAGENT_CLOSE_CHANNEL:
			{
				unsubscribe_channel(dgi.read_uint64());
			}
			break;
			case CLIENTAGENT_ADD_POST_REMOVE:
			{
				add_post_remove(dgi.read_string());
			}
			break;
			case CLIENTAGENT_CLEAR_POST_REMOVE:
			{
				clear_post_removes();
			}
			break;
			case STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED:
			case STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER:
			{
				uint16_t dc_id = dgi.read_uint16();
				uint32_t do_id = dgi.read_uint32();
				uint32_t parent = dgi.read_uint32();
				uint32_t zone = dgi.read_uint32();
				if(dist_objs.find(do_id) == dist_objs.end() || dist_objs[do_id].refcount == 0)
				{
					DistributedObject obj;
					obj.id = do_id;
					obj.dcc = gDCF->get_class(dc_id);
					obj.parent = parent;
					obj.refcount = 0;
					obj.zone = zone;
					dist_objs[do_id] = obj;
				}
				dist_objs[do_id].refcount++;

				Datagram resp;
				if(msgtype == STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED)
				{
					resp.add_uint16(CLIENT_CREATE_OBJECT_REQUIRED);
				}
				else
				{
					resp.add_uint16(CLIENT_CREATE_OBJECT_REQUIRED_OTHER);
				}
				resp.add_uint32(parent);
				resp.add_uint32(zone);
				resp.add_uint16(dc_id);
				resp.add_uint32(do_id);
				resp.add_data(dgi.read_remainder());
				network_send(resp);
			}
			break;
			case STATESERVER_OBJECT_QUERY_ZONE_ALL_DONE:
			{
				uint32_t parent = dgi.read_uint32();
				uint16_t n_zones = dgi.read_uint16();
				std::vector<uint32_t> zones(0);
				zones.reserve(n_zones);
				for(uint16_t i = 0; i < n_zones; ++i)
				{
					zones.insert(zones.end(), dgi.read_uint32());
				}
				for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
				{
					Interest &i = it->second;
					if(!i.is_ready())
					{
						for(auto it2 = i.zones.begin();  it2 != i.zones.end(); ++it2)
						{
							if(!it2->second)
							{
								for(auto it3 = zones.begin(); it3 != zones.end(); ++it3)
								{
									if(it2->first == *it3)
									{
										it2->second = true;
										break;
									}
								}
							}
						}
						if(i.is_ready())
						{
							Datagram resp;
							resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
							resp.add_uint16(it->first);
							resp.add_uint32(i.context);
							network_send(resp);
						}
					}
				}
			}
			break;
			case STATESERVER_OBJECT_CHANGE_ZONE:
			{
				uint32_t do_id = dgi.read_uint32();
				uint32_t n_parent = dgi.read_uint32();
				uint32_t n_zone = dgi.read_uint32();
				uint32_t o_parent = dgi.read_uint32();
				uint32_t o_zone = dgi.read_uint32();
				bool disable = true;
				for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
				{
					Interest &i = it->second;
					for(auto it2 = i.zones.begin(); it2 != i.zones.end(); ++it2)
					{
						if(it2->first == n_zone)
						{
							disable = false;
							break;
						}
					}
				}

				if(dist_objs.find(do_id) != dist_objs.end())
				{
					dist_objs[do_id].zone = n_zone;
					if(disable)
					{
						dist_objs[do_id].refcount--;
					}
				}

				Datagram resp;
				if(disable)
				{
					resp.add_uint16(CLIENT_OBJECT_DISABLE);
					resp.add_uint32(do_id);
				}
				else
				{
					resp.add_uint16(CLIENT_OBJECT_LOCATION);
					resp.add_uint32(do_id);
					resp.add_uint32(n_parent);
					resp.add_uint32(n_zone);
				}
				network_send(resp);
			}
			break;
			default:
				m_log->error() << "Recv'd unk server msgtype " << msgtype << std::endl;
			}
		}

	private:
		virtual void network_datagram(Datagram &dg)
		{
			if(dg.get_buf_end() == 0)
			{
				m_log->error() << "0-length DG" << std::endl;
				send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM, "0-length");
				return;
			}

			try
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
						handle_authenticated(dg);
						break;
				}
			}
			catch(std::exception &e)
			{
				m_log->error() << "Exception while parsing client dg. DCing for truncated "
					"e.what() " << e.what() << std::endl;
				send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM);
				return;
			}
		}

		void send_disconnect(uint16_t reason, const std::string &error_string = "")
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
				send_disconnect(CLIENT_DISCONNECT_NO_HELLO);
				return;
			}

			uint32_t dc_hash = dgi.read_uint32();
			const static uint32_t expected_hash = gDCF->get_hash();
			if(dc_hash != expected_hash)
			{
				m_log->error() << m_client_name << "Wrong DC hash. Got: "
					<< std::hex << dc_hash << " expected " <<
					gDCF->get_hash() << std::dec << std::endl;
				send_disconnect(CLIENT_DISCONNECT_BAD_DCHASH);
				return;
			}

			std::string version = dgi.read_string();
			const static std::string expected_version = server_version.get_rval(m_roleconfig);
			if(version != expected_version)
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
				if(uberdogs.find(do_id) == uberdogs.end() || !uberdogs[do_id].anonymous)
				{
					send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION);
					return;
				}
				dcc = uberdogs[do_id].dcc;

				DCField *field = dcc->get_field_by_index(field_id);
				if(!field)
				{
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, "field does not exist for object");
					return;
				}
				if(!field->is_clsend())
				{
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, "field does not have clsend");
					return;
				}

				std::string data;
				dgi.unpack_field(field, data);//if an exception occurs it will be handled
				//and client will be dc'd for truncated datagram

				Datagram resp;
				resp.add_server_header(do_id, m_channel, STATESERVER_OBJECT_UPDATE_FIELD);
				resp.add_uint32(do_id);
				resp.add_uint16(field_id);
				if(data.length() > 65535- resp.get_buf_end())
				{
					send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM);
					return;
				}
				resp.add_data(data);
				send(resp);
			}
			break;
			default:
				m_log->warning() << m_client_name << "Recv'd unk msg type " << msg_type << std::endl;
				send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE);
				return;
			}
			if(dgi.tell() < dg.get_buf_end())
			{
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM);
				return;
			}
		}

		void handle_authenticated(Datagram &dg)
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
				if(uberdogs.find(do_id) != uberdogs.end())
				{
					dcc = uberdogs[do_id].dcc;
				}
				if(!dcc)
				{
					if(dist_objs.find(do_id) != dist_objs.end())
					{
						dcc = dist_objs[do_id].dcc;
					}
					else
					{
						send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT);
						return;
					}
				}

				DCField *field = dcc->get_field_by_index(field_id);
				if(!field)
				{
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, "field does not exist for object");
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

				if(!field->is_clsend() && !(is_owned && field->is_ownsend()))
				{
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, "field does not have clsend");
					return;
				}

				std::string data;
				dgi.unpack_field(field, data);//if an exception occurs it will be handled
				//and client will be dc'd for truncated datagram

				Datagram resp;
				resp.add_server_header(do_id, m_channel, STATESERVER_OBJECT_UPDATE_FIELD);
				resp.add_uint32(do_id);
				resp.add_uint16(field_id);
				if(data.length() > 65535- resp.get_buf_end())
				{
					send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM);
					return;
				}
				resp.add_data(data);
				send(resp);
			}
			break;
			case CLIENT_OBJECT_LOCATION:
			{
				uint32_t do_id = dgi.read_uint32();
				if(dist_objs.find(do_id) == dist_objs.end())
				{
					send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT);
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
					send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_RELOCATE);
					return;
				}

				//TODO: Finish this
				dgi.read_remainder();
			}
			break;
			case CLIENT_ADD_INTEREST:
			{
				uint16_t interest_id = dgi.read_uint16();
				uint32_t context = dgi.read_uint32();
				uint32_t parent = dgi.read_uint32();
				if(m_interests.find(interest_id) != m_interests.end())
				{
					m_log->warning() << m_client_name << "Duplicate interest handle, dropping client" << std::endl;
					send_disconnect(CLIENT_DISCONNECT_GENERIC, "Duplicate interest handle");
				}
				Interest i;
				i.context = context;
				i.parent = parent;

				i.zones.reserve((dg.get_buf_end()-dgi.tell())/sizeof(uint32_t));
				for(uint16_t p = dgi.tell(); p != dg.get_buf_end(); p = dgi.tell())
				{
					uint32_t zone = dgi.read_uint32();
					i.zones.insert(i.zones.end(), std::pair<uint32_t, bool>(zone, false));
					subscribe_channel(LOCATION2CHANNEL(parent, zone));
				}

				m_interests[interest_id] = i;

				Datagram resp;
				resp.add_server_header(parent, m_channel, STATESERVER_OBJECT_QUERY_ZONE_ALL);
				resp.add_uint32(parent);
				resp.add_uint16(i.zones.size());
				for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
				{
					resp.add_uint32(it->first);
				}
				send(resp);
			}
			break;
			case CLIENT_REMOVE_INTEREST:
			{
				uint16_t id = dgi.read_uint16();
				uint32_t context = 0;
				if(dgi.tell() < dg.get_buf_end())
				{
					context = dgi.read_uint32();
				}
				if(m_interests.find(id) == m_interests.end())
				{
					send_disconnect(CLIENT_DISCONNECT_GENERIC, "Tried to remove a non-existing intrest");
					return;
				}
				Interest &i = m_interests[id];
				std::vector<uint32_t> removed_zones(0);
				removed_zones.reserve(i.zones.size());
				for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
				{
					uint32_t zone = it->first;
					bool found = false;
					for(auto it2 = m_interests.begin(); it2 != m_interests.end(); ++it2)
					{
						for(auto it3 = it2->second.zones.begin(); it3 != it2->second.zones.begin(); ++it3)
						{
							if(it3->first == zone)
							{
								found = true;
								break;
							}
						}
						if(found)
						{
							break;
						}
					}
					if(!found)
					{
						removed_zones.insert(removed_zones.end(), zone);
						unsubscribe_channel(LOCATION2CHANNEL(i.parent, zone));
					}
				}
				for(auto it = dist_objs.begin(); it != dist_objs.end(); ++it)
				{
					if(it->second.parent == i.parent)
					{
						bool found = false;
						for(auto it2 = removed_zones.begin(); it2 != removed_zones.end(); ++it2)
						{
							if(it->second.zone == *it2)
							{
								found = true;
								break;
							}
						}
						if(found)
						{
							Datagram resp;
							resp.add_uint16(CLIENT_OBJECT_DISABLE);
							resp.add_uint32(it->second.id);
							network_send(resp);
							it->second.refcount--;
						}
					}
				}

				if(context)
				{
					Datagram resp;
					resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
					resp.add_uint16(id);
					resp.add_uint32(context);
					network_send(resp);
				}
			}
			break;
			default:
				m_log->warning() << m_client_name << "Recv'd unk msg type " << msg_type << std::endl;
				send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE);
				return;
			}
			if(dgi.tell() < dg.get_buf_end())
			{
				send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM);
				return;
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
		ChannelTracker m_ct;
	public:
		ClientAgent(RoleConfig roleconfig) : Role(roleconfig),
			m_acceptor(NULL), m_ct(min_channel.get_rval(roleconfig), max_channel.get_rval(roleconfig))
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

			if(uberdogs.empty())
			{
				YAML::Node udnodes = gConfig->copy_node()["uberdogs"];
				if(!udnodes.IsNull())
				{
					for(auto it = udnodes.begin(); it != udnodes.end(); ++it)
					{
						YAML::Node udnode = *it;
						Uberdog ud;
						ud.dcc = gDCF->get_class_by_name(udnode["class"].as<std::string>());
						if(!ud.dcc)
						{
							m_log->fatal() << "DCClass " << udnode["class"].as<std::string>()
								<< "Does not exist!" << std::endl;
							exit(1);
						}
						ud.anonymous = udnode["anonymous"].as<bool>();
						uberdogs[udnode["id"].as<uint32_t>()] = ud;
					}
				}
			}

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
			new Client(socket, m_log, m_roleconfig, &m_ct); //It deletes itsself when connection is lost
			start_accept();
		}

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{

		}
};

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
