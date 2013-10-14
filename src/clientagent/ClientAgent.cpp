#include "ClientMessages.h"
#include "ClientFactory.h"
#include "ClientAgent.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

#include <queue>
#include <set>
#include <list>
#include <algorithm>

using boost::asio::ip::tcp;

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");
static ConfigVariable<std::string> server_version("version", "dev");
static ConfigVariable<channel_t> min_channel("channels/min", 0);
static ConfigVariable<channel_t> max_channel("channels/max", 0);

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
		std::queue<channel_t> m_unused_channels;
	public:
		ChannelTracker(channel_t min, channel_t max) : m_next(min),
			m_max(max), m_unused_channels()
		{
		}

		channel_t alloc_channel()
		{
			channel_t ret;
			if(m_next <= m_max)
			{
				return m_next++;
			}
			else
			{
				if(!m_unused_channels.empty())
				{
					ret = m_unused_channels.front();
					m_unused_channels.pop();
				}
			}
			return 0;
		}

		void free_channel(channel_t channel)
		{
			m_unused_channels.push(channel);
		}
};

struct Uberdog
{
	DCClass *dcc;
	bool anonymous;
};

std::map<uint32_t, Uberdog> uberdogs;

Client::Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig,
	ChannelTracker *ct) : NetworkClient(socket), m_state(CLIENT_STATE_NEW),
		m_roleconfig(roleconfig), m_ct(ct), m_channel(0),
		m_is_channel_allocated(true), m_owned_objects(), m_seen_objects(),
		m_allocated_channel(0), m_interests()
{
	m_channel = m_ct->alloc_channel();
	if(!m_channel)
	{
		send_disconnect(CLIENT_DISCONNECT_GENERIC, "Client capacity reached");
		return;
	}
	m_allocated_channel = m_channel;
	subscribe_channel(m_channel);
	std::stringstream ss;
	ss << "Client (" << socket->remote_endpoint().address().to_string() << ":"
		<< socket->remote_endpoint().port() << ")";
	m_log = new LogCategory("client", ss.str());
}

Client::~Client()
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
void Client::handle_datagram(Datagram &dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
	case CLIENTAGENT_DISCONNECT:
	{
		uint16_t reason = dgi.read_uint16();
		std::string error_string = dgi.read_string();
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
		if(sender != m_channel)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_UPDATE_FIELD);
			resp.add_data(dgi.read_remainder());
			network_send(resp);
		}
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
			obj.dcc = g_dcf->get_class(dc_id);
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
		uint32_t parent = dgi.read_uint32();
		uint32_t zone = dgi.read_uint32();
		uint16_t dc_id = dgi.read_uint16();
		uint32_t do_id = dgi.read_uint32();
		if(m_owned_objects.find(do_id) != m_owned_objects.end() ||
		   m_seen_objects.find(do_id) != m_seen_objects.end())
		{
			return;
		}
		if(dist_objs.find(do_id) == dist_objs.end() || dist_objs[do_id].refcount == 0)
		{
			DistributedObject obj;
			obj.id = do_id;
			obj.dcc = g_dcf->get_class(dc_id);
			obj.parent = parent;
			obj.refcount = 0;
			obj.zone = zone;
			dist_objs[do_id] = obj;
		}
		dist_objs[do_id].refcount++;
		m_seen_objects.insert(do_id);

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
		if(disable && m_owned_objects.find(do_id) == m_owned_objects.end())
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

void Client::network_datagram(Datagram &dg)
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
		send_disconnect(CLIENT_DISCONNECT_TRUNCATED_DATAGRAM, "Datagram unexpectedly ended while iterating.");
		return;
	}

	if(dgi.get_remaining())
	{
		send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Datagram contains excess data.", true);
		return;
	}
}

void Client::send_disconnect(uint16_t reason, const std::string &error_string, bool security)
{
	if(is_connected())
	{
		(security ? m_log->security() : m_log->error())
				<< "Terminating client connection (" << reason << "): "
				<< error_string << std::endl;

		Datagram resp;
		resp.add_uint16(CLIENT_GO_GET_LOST);
		resp.add_uint16(reason);
		resp.add_string(error_string);
		network_send(resp);
		do_disconnect();
	}
}

DCClass *Client::lookup_object(uint32_t do_id)
{
	// First see if it's an UberDOG:
	if(uberdogs.find(do_id) != uberdogs.end())
	{
		return uberdogs[do_id].dcc;
	}

	// Next, check the shared object cache, but this client only knows about it
	// if it occurs in m_seen_objects or m_owned_objects:
	if(m_owned_objects.find(do_id) != m_owned_objects.end() ||
	   m_seen_objects.find(do_id) != m_seen_objects.end())
	{
		if(dist_objs.find(do_id) != dist_objs.end())
		{
			return dist_objs[do_id].dcc;
		}
	}

	// We're at the end of our rope; we have no clue what this object is.
	return NULL;
}

//Only handles one message type, so it does not need to be split up.
void Client::handle_pre_hello(DatagramIterator &dgi)
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
	const static std::string expected_version = server_version.get_rval(m_roleconfig);
	if(version != expected_version)
	{
		std::stringstream ss;
		ss << "Client version mismatch: server=" << expected_version << ", client=" << version;
		send_disconnect(CLIENT_DISCONNECT_BAD_VERSION, ss.str());
		return;
	}

	Datagram resp;
	resp.add_uint16(CLIENT_HELLO_RESP);
	network_send(resp);

	m_state = CLIENT_STATE_ANONYMOUS;
}

void Client::handle_pre_auth(DatagramIterator &dgi)
{
	uint16_t msg_type = dgi.read_uint16();
	bool should_die = false;
	switch(msg_type)
	{
	case CLIENT_OBJECT_UPDATE_FIELD:
	{
		should_die = handle_client_object_update_field(dgi);
	}
	break;
	default:
		std::stringstream ss;
		ss << "Message type " << msg_type << " not allowed prior to authentication.";
		send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
		return;
	}
	if(should_die)
	{
		return;
	}
}

void Client::handle_authenticated(DatagramIterator &dgi)
{
	uint16_t msg_type = dgi.read_uint16();
	bool should_die = false;
	switch(msg_type)
	{
	case CLIENT_OBJECT_UPDATE_FIELD:
		should_die = handle_client_object_update_field(dgi);
		break;
	case CLIENT_OBJECT_LOCATION:
		should_die = handle_client_object_location(dgi);
		break;
	case CLIENT_ADD_INTEREST:
		should_die = handle_client_add_interest(dgi);
		break;
	case CLIENT_REMOVE_INTEREST:
		should_die = handle_client_remove_interest(dgi);
		break;
	default:
		std::stringstream ss;
		ss << "Message type " << msg_type << " not valid.";
		send_disconnect(CLIENT_DISCONNECT_INVALID_MSGTYPE, ss.str(), true);
		return;
	}

	if(should_die)
	{
		return;
	}
}

//returns new zones
std::list<uint32_t> Client::add_interest(Interest &i)
{
	std::list<uint32_t> new_zones;
	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		new_zones.insert(new_zones.end(), it->first);
	}
	for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
	{
		if(it->second.parent != i.parent)
		{
			continue;
		}
		for(auto it2 = it->second.zones.begin(); it2 != it->second.zones.end(); ++it2)
		{
			new_zones.remove(it2->first);
		}
	}
	return new_zones;
}

void Client::request_zone_objects(uint32_t parent, std::list<uint32_t> new_zones)
{
	Datagram resp;
	resp.add_server_header(parent, m_channel, STATESERVER_OBJECT_QUERY_ZONE_ALL);
	resp.add_uint32(parent);
	resp.add_uint16(new_zones.size());
	for(auto it = new_zones.begin(); it != new_zones.end(); ++it)
	{
		resp.add_uint32(*it);
		subscribe_channel(LOCATION2CHANNEL(parent, *it));
	}
	send(resp);
}

void Client::remove_interest(Interest &i, uint32_t id)
{
	std::vector<uint32_t> removed_zones(0);
	removed_zones.reserve(i.zones.size());
	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		uint32_t zone = it->first;
		bool found = false;
		for(auto it2 = m_interests.begin(); it2 != m_interests.end(); ++it2)
		{
			if(it2->first == id || it2->second.parent != i.parent)
			{
				continue;
			}
			for(auto it3 = it2->second.zones.begin(); it3 != it2->second.zones.end(); ++it3)
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
			if(found && m_owned_objects.find(it->second.id) == m_owned_objects.end())
			{
				Datagram resp;
				resp.add_uint16(CLIENT_OBJECT_DISABLE);
				resp.add_uint32(it->second.id);
				network_send(resp);
				it->second.refcount--;
			}
		}
	}
}

void Client::alter_interest(Interest &i, uint16_t id)
{
	Interest &other = m_interests[id];
	if(other.parent != i.parent)
	{
		remove_interest(other, id);
		std::list<uint32_t> new_zones = add_interest(i);
		m_interests[id] = i;
		request_zone_objects(i.parent, new_zones);
		return;
	}
	std::list<uint32_t> new_zones;
	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		bool found = false;
		for(auto it2 = other.zones.begin(); it2 != other.zones.end(); ++it2)
		{
			if(it->first == it2->first)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			new_zones.insert(new_zones.end(), it->first);
		}
	}

	std::vector<uint32_t> removed_zones;
	removed_zones.reserve(other.zones.size());
	for(auto it = other.zones.begin(); it != other.zones.end(); ++it)
	{
		bool found = false;
		for(auto it2 = i.zones.begin(); it2 != i.zones.end(); ++it2)
		{
			if(it2->first == it->first)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			removed_zones.insert(removed_zones.end(), it->first);
		}
	}

	Interest temp;
	temp.parent = i.parent;
	temp.context = i.context;
	for(auto it = removed_zones.begin(); it != removed_zones.end(); ++it)
	{
		temp.zones.insert(temp.zones.end(), std::pair<uint32_t, bool>(*it, false));
	}
	remove_interest(temp, id);

	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		for(auto it2 = other.zones.begin(); it2 != other.zones.end(); ++it2)
		{
			if(it->first == it2->first)
			{
				it->second = it2->second;
				break;
			}
		}
	}
	if(i.is_ready())
	{
		Datagram resp;
		resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
		resp.add_uint16(id);
		resp.add_uint32(i.context);
		network_send(resp);
	}
	m_interests[id] = i;
	// This should be done last -- if the SS is in the same process,
	// the response may come back instantly.
	if(!new_zones.empty())
	{
		request_zone_objects(i.parent, new_zones);
	}
}

bool Client::handle_client_object_update_field(DatagramIterator &dgi)
{
	uint32_t do_id = dgi.read_uint32();
	uint16_t field_id = dgi.read_uint16();

	DCClass *dcc = lookup_object(do_id);

	// If the class couldn't be found, error out:
	if(!dcc)
	{
		std::stringstream ss;
		ss << "Client tried to send update to nonexistent object " << do_id;
		send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
		return true;
	}

	// If the client is not in the ESTABLISHED state, it may only send updates
	// to anonymous UberDOGs.
	if(m_state != CLIENT_STATE_ESTABLISHED)
	{
		if(uberdogs.find(do_id) == uberdogs.end() || !uberdogs[do_id].anonymous)
		{
			std::stringstream ss;
			ss << "Client tried to send update to non-anonymous object "
			   << dcc->get_name() << "(" << do_id << ")";
			send_disconnect(CLIENT_DISCONNECT_ANONYMOUS_VIOLATION, ss.str(), true);
			return true;
		}
	}


	DCField *field = dcc->get_field_by_index(field_id);
	if(!field)
	{
		std::stringstream ss;
		ss << "Client tried to send update for nonexistent field " << field_id << " to object "
		   << dcc->get_name() << "(" << do_id << ")";
		send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
		return true;
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
		std::stringstream ss;
		ss << "Client tried to send update for non-sendable field: "
		   << dcc->get_name() << "(" << do_id << ")." << field->get_name();
		send_disconnect(CLIENT_DISCONNECT_FORBIDDEN_FIELD, ss.str(), true);
		return true;
	}

	std::vector<uint8_t> data;
	dgi.unpack_field(field, data);//if an exception occurs it will be handled
	//and client will be dc'd for truncated datagram

	Datagram resp;
	resp.add_server_header(do_id, m_channel, STATESERVER_OBJECT_UPDATE_FIELD);
	resp.add_uint32(do_id);
	resp.add_uint16(field_id);
	if(data.size() > 65535u-resp.size())
	{
		send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Field update too large to be routed on MD.", true);
		return true;
	}
	resp.add_data(data);
	send(resp);
	return false;
}

bool Client::handle_client_object_location(DatagramIterator &dgi)
{
	uint32_t do_id = dgi.read_uint32();
	if(dist_objs.find(do_id) == dist_objs.end())
	{
		std::stringstream ss;
		ss << "Client tried to manipulate unknown object " << do_id;
		send_disconnect(CLIENT_DISCONNECT_MISSING_OBJECT, ss.str(), true);
		return true;
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
		return true;
	}

	Datagram dg(do_id, m_channel, STATESERVER_OBJECT_SET_ZONE);
	dg.add_uint32(dgi.read_uint32()); // Parent
	dg.add_uint32(dgi.read_uint32()); // Zone
	send(dg);
	return false;
}

bool Client::handle_client_add_interest(DatagramIterator &dgi)
{
	uint16_t interest_id = dgi.read_uint16();
	uint32_t context = dgi.read_uint32();
	uint32_t parent = dgi.read_uint32();
	Interest i;
	i.context = context;
	i.parent = parent;

	i.zones.reserve(dgi.get_remaining()/sizeof(uint32_t));
	while(dgi.get_remaining())
	{
		uint32_t zone = dgi.read_uint32();
		i.zones.insert(i.zones.end(), std::pair<uint32_t, bool>(zone, false));
	}

	if(m_interests.find(interest_id) != m_interests.end())
	{
		alter_interest(i, interest_id);
		return false;//alter_interest takes care of the rest
	}
	std::list<uint32_t> new_zones = add_interest(i);
	m_interests[interest_id] = i;

	if(!new_zones.empty())
	{
		request_zone_objects(i.parent, new_zones);
	}
	else
	{
		Datagram resp;
		resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
		resp.add_uint16(interest_id);
		resp.add_uint32(context);
		network_send(resp);
	}
	return false;
}

bool Client::handle_client_remove_interest(DatagramIterator &dgi)
{
	uint16_t id = dgi.read_uint16();
	uint32_t context = 0;
	if(dgi.get_remaining())
	{
		context = dgi.read_uint32();
	}
	if(m_interests.find(id) == m_interests.end())
	{
		send_disconnect(CLIENT_DISCONNECT_GENERIC, "Tried to remove a non-existing intrest", true);
		return true;
	}
	Interest &i = m_interests[id];
	remove_interest(i, id);
	m_interests.erase(m_interests.find(id));

	if(context)
	{
		Datagram resp;
		resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
		resp.add_uint16(id);
		resp.add_uint32(context);
		network_send(resp);
	}
	return false;
}

void Client::network_disconnect()
{
	delete this;
}

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
			ClientFactory::get_singleton().create(socket, m_log, m_roleconfig, &m_ct);
			start_accept();
		}

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{

		}
};


static ClientType<Client> client_type(0);

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
