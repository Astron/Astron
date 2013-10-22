#include "Client.h"
#include "ClientMessages.h"
#include "ClientFactory.h"

#include "core/global.h"

Client::Client(boost::asio::ip::tcp::socket *socket, LogCategory *log, const std::string &server_version,
	ChannelTracker *ct) : NetworkClient(socket), m_state(CLIENT_STATE_NEW),
		m_server_version(server_version), m_ct(ct), m_channel(0), m_allocated_channel(0),
		m_is_channel_allocated(true), m_clean_disconnect(false), m_next_context(0),
		m_owned_objects(), m_seen_objects(), m_interests(), m_pending_interests()
{
	m_channel = m_ct->alloc_channel();
	if(!m_channel)
	{
		send_disconnect(CLIENT_DISCONNECT_GENERIC, "Client capacity reached");
		return;
	}
	m_allocated_channel = m_channel;
	subscribe_channel(m_channel);
	subscribe_channel(BCHAN_CLIENTS);
	std::stringstream ss;
	ss << "Client (" << socket->remote_endpoint().address().to_string() << ":"
		<< socket->remote_endpoint().port() << ")";
	m_log = new LogCategory("client", ss.str());

	std::list<std::string> event;
	event.push_back("client-connected");
	ss.str("");
	ss << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port();
	event.push_back(ss.str());
	ss.str("");
	ss << socket->local_endpoint().address().to_string() << ":" << socket->local_endpoint().port();
	event.push_back(ss.str());
	send_event(event);
}

Client::~Client()
{
	m_ct->free_channel(m_allocated_channel);
}

//for participant interface
void Client::handle_datagram(Datagram &dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
	case CLIENTAGENT_EJECT:
	{
		uint16_t reason = dgi.read_uint16();
		std::string error_string = dgi.read_string();
		send_disconnect(reason, error_string);
		return;
	}
	break;
	case CLIENTAGENT_DROP:
	{
		m_clean_disconnect = true;
		do_disconnect();
		return;
	}
	break;
	case CLIENTAGENT_SET_STATE:
	{
		m_state = (ClientState)dgi.read_uint16();
	}
	break;
	case STATESERVER_OBJECT_SET_FIELD:
	{
		uint32_t do_id = dgi.read_uint32();
		if(!lookup_object(do_id))
		{
			m_log->warning() << "Received server-side field update for unknown object " << do_id << std::endl;
			return;
		}
		if(sender != m_channel)
		{
			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_SET_FIELD);
			resp.add_uint32(do_id);
			resp.add_data(dgi.read_remainder());
			network_send(resp);
		}
	}
	break;
	case STATESERVER_OBJECT_DELETE_RAM:
	{
		uint32_t do_id = dgi.read_uint32();
		if(!lookup_object(do_id))
		{
			m_log->warning() << "Received server-side object delete for unknown object " << do_id << std::endl;
			return;
		}

		if(m_seen_objects.find(do_id) != m_seen_objects.end())
		{
			m_seen_objects.erase(do_id);

			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING);
			resp.add_uint32(do_id);
			network_send(resp);
		}

		if(m_owned_objects.find(do_id) != m_owned_objects.end())
		{
			m_owned_objects.erase(do_id);

			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING_OWNER);
			resp.add_uint32(do_id);
			network_send(resp);
		}

		m_dist_objs.erase(do_id);
	}
	break;
	case STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER:
	{
		uint32_t do_id = dgi.read_uint32();
		uint32_t parent = dgi.read_uint32();
		uint32_t zone = dgi.read_uint32();
		uint16_t dc_id = dgi.read_uint16();
		m_owned_objects.insert(do_id);

		if(m_dist_objs.find(do_id) == m_dist_objs.end())
		{
			VisibleObject obj;
			obj.id = do_id;
			obj.parent = parent;
			obj.zone = zone;
			obj.dcc = g_dcf->get_class(dc_id);
			m_dist_objs[do_id] = obj;
		}

		Datagram resp;
		resp.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER_OWNER);
		resp.add_uint32(do_id);
		resp.add_uint32(parent);
		resp.add_uint32(zone);
		resp.add_uint16(dc_id);
		resp.add_data(dgi.read_remainder());
		network_send(resp);
	}
	break;
	case CLIENTAGENT_SET_CLIENT_ID:
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
	case CLIENTAGENT_CLEAR_POST_REMOVES:
	{
		clear_post_removes();
	}
	break;
	case STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED:
	case STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER:
	{
		uint32_t do_id = dgi.read_uint32();
		uint32_t parent = dgi.read_uint32();
		uint32_t zone = dgi.read_uint32();
		uint16_t dc_id = dgi.read_uint16();
		if(m_owned_objects.find(do_id) != m_owned_objects.end() ||
		   m_seen_objects.find(do_id) != m_seen_objects.end())
		{
			return;
		}
		if(m_dist_objs.find(do_id) == m_dist_objs.end())
		{
			VisibleObject obj;
			obj.id = do_id;
			obj.dcc = g_dcf->get_class(dc_id);
			obj.parent = parent;
			obj.zone = zone;
			m_dist_objs[do_id] = obj;
		}
		m_seen_objects.insert(do_id);

		Datagram resp;
		if(msgtype == STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED)
		{
			resp.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED);
		}
		else
		{
			resp.add_uint16(CLIENT_ENTER_OBJECT_REQUIRED_OTHER);
		}
		resp.add_uint32(do_id);
		resp.add_uint32(parent);
		resp.add_uint32(zone);
		resp.add_uint16(dc_id);
		resp.add_data(dgi.read_remainder());
		network_send(resp);

		// TODO: This is a tad inefficient as it checks every pending interest.
		// In practice, there shouldn't be many add-interest operations active
		// at once, however.
		std::list<uint32_t> deferred_deletes;
		for(auto it = m_pending_interests.begin(); it != m_pending_interests.end(); ++it)
		{
			if(it->second->is_ready(m_dist_objs))
			{
				Datagram resp;
				resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
				resp.add_uint32(it->second->m_client_context);
				resp.add_uint16(it->second->m_interest_id);
				network_send(resp);

				deferred_deletes.push_back(it->first);
			}
		}
		for(auto it = deferred_deletes.begin(); it != deferred_deletes.end(); ++it)
		{
			m_pending_interests.erase(*it);
		}
	}
	break;
    case STATESERVER_OBJECT_GET_ZONES_COUNT_RESP:
	{
		uint32_t context = dgi.read_uint32();
		uint32_t count = dgi.read_uint32();

		if(m_pending_interests.find(context) == m_pending_interests.end())
		{
			m_log->error() << "Received GET_ZONES_COUNT_RESP for unknown context "
			               << context << std::endl;
			return;
		}

		m_pending_interests[context]->store_total(count);

		if(m_pending_interests[context]->is_ready(m_dist_objs))
		{
			Datagram resp;
			resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
			resp.add_uint32(m_pending_interests[context]->m_client_context);
			resp.add_uint16(m_pending_interests[context]->m_interest_id);
			network_send(resp);

			m_pending_interests.erase(context);
		}
	}
	break;
	case STATESERVER_OBJECT_CHANGING_LOCATION:
	{
		uint32_t do_id = dgi.read_uint32();
		uint32_t n_parent = dgi.read_uint32();
		uint32_t n_zone = dgi.read_uint32();
		dgi.read_uint32(); // Old parent; we don't care about this.
		dgi.read_uint32(); // Old zone; we don't care about this.
		bool disable = true;
		for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
		{
			Interest &i = it->second;
			for(auto it2 = i.zones.begin(); it2 != i.zones.end(); ++it2)
			{
				if(*it2 == n_zone)
				{
					disable = false;
					break;
				}
			}
		}

		if(m_dist_objs.find(do_id) != m_dist_objs.end())
		{
			m_dist_objs[do_id].parent = n_parent;
			m_dist_objs[do_id].zone = n_zone;
		}

		Datagram resp;
		if(disable && m_owned_objects.find(do_id) == m_owned_objects.end())
		{
			resp.add_uint16(CLIENT_OBJECT_LEAVING);
			resp.add_uint32(do_id);

			m_seen_objects.erase(do_id);
			m_dist_objs.erase(do_id);
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

void Client::send_event(const std::list<std::string> &event)
{
	Datagram dg;

	std::stringstream ss;
	ss << "Client:" << m_allocated_channel;
	dg.add_string(ss.str());

	for(auto it = event.begin(); it != event.end(); ++it)
	{
		dg.add_string(*it);
	}

	g_eventsender.send(dg);
}

void Client::send_disconnect(uint16_t reason, const std::string &error_string, bool security)
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
		send_event(event);

		Datagram resp;
		resp.add_uint16(CLIENT_EJECT);
		resp.add_uint16(reason);
		resp.add_string(error_string);
		network_send(resp);
		m_clean_disconnect = true;
		do_disconnect();
	}
}

DCClass *Client::lookup_object(uint32_t do_id)
{
	// First see if it's an UberDOG:
	if(g_uberdogs.find(do_id) != g_uberdogs.end())
	{
		return g_uberdogs[do_id].dcc;
	}

	// Next, check the object cache, but this client only knows about it
	// if it occurs in m_seen_objects or m_owned_objects:
	if(m_owned_objects.find(do_id) != m_owned_objects.end() ||
	   m_seen_objects.find(do_id) != m_seen_objects.end())
	{
		if(m_dist_objs.find(do_id) != m_dist_objs.end())
		{
			return m_dist_objs[do_id].dcc;
		}
	}

	// We're at the end of our rope; we have no clue what this object is.
	return NULL;
}

std::list<Interest> Client::lookup_interests(uint32_t parent_id, uint32_t zone_id)
{
	std::list<Interest> interests;
	for(auto it = m_interests.begin(); it != m_interests.end(); ++it)
	{
		if(parent_id == it->second.parent && (it->second.zones.find(zone_id) != it->second.zones.end()))
		{
			interests.push_back(it->second);
		}
	}
	return interests;
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
	if(version != m_server_version)
	{
		std::stringstream ss;
		ss << "Client version mismatch: server=" << m_server_version << ", client=" << version;
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
	switch(msg_type)
	{
	case CLIENT_DISCONNECT:
	{
		std::list<std::string> event;
		event.push_back("client-disconnected");
		send_event(event);

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

void Client::handle_authenticated(DatagramIterator &dgi)
{
	uint16_t msg_type = dgi.read_uint16();
	switch(msg_type)
	{
	case CLIENT_DISCONNECT:
	{
		std::list<std::string> event;
		event.push_back("client-disconnected");
		send_event(event);

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

void Client::close_zones(uint32_t parent, const std::unordered_set<uint32_t> &killed_zones)
{
	// Kill off all objects that are in the matched parent/zones:

	std::list<uint32_t> to_remove;
	for(auto it = m_dist_objs.begin(); it != m_dist_objs.end(); ++it)
	{
		if(it->second.parent != parent)
		{
			// Object does not belong to the parent in question; ignore.
			continue;
		}

		if(killed_zones.find(it->second.zone) != killed_zones.end())
		{
			if(m_owned_objects.find(it->second.id) != m_owned_objects.end())
			{
				// Owned objects are always zone-visible. I think.
				// TODO: Is this assumption correct?
				continue;
			}

			Datagram resp;
			resp.add_uint16(CLIENT_OBJECT_LEAVING);
			resp.add_uint32(it->second.id);
			network_send(resp);

			m_seen_objects.erase(it->second.id);
			to_remove.push_back(it->second.id);
		}
	}

	for(auto it = to_remove.begin(); it != to_remove.end(); ++it)
	{
		m_dist_objs.erase(*it);
	}

	// Close all of the channels:
	for(auto it = killed_zones.begin(); it != killed_zones.end(); ++it)
	{
		unsubscribe_channel(LOCATION2CHANNEL(parent, *it));
	}
}

void Client::add_interest(Interest &i, uint32_t context)
{
	std::unordered_set<uint32_t> new_zones;

	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		if(lookup_interests(i.parent, *it).empty())
		{
			new_zones.insert(*it);
		}
	}

	if(m_interests.find(i.id) != m_interests.end())
	{
		// This is an already-open interest that is actually being altered.
		// Therefore, we need to delete the objects that the client can see
		// through this interest only.

		Interest previous_interest = m_interests[i.id];
		std::unordered_set<uint32_t> killed_zones;

		for(auto it = previous_interest.zones.begin(); it != previous_interest.zones.end(); ++it)
		{
			if(lookup_interests(previous_interest.parent, *it).size() > 1)
			{
				// An interest other than the altered one can see this parent/zone,
				// so we don't care about it.
				continue;
			}

			// If we've gotten here: parent,*it is unique, so if the new interest
			// doesn't cover it, we add it to the killed zones.
			if(i.parent != previous_interest.parent || i.zones.find(*it) == i.zones.end())
			{
				killed_zones.insert(*it);
			}
		}

		// Now that we know what zones to kill, let's get to it:
		close_zones(previous_interest.parent, killed_zones);
	}
	m_interests[i.id] = i;

	if(new_zones.empty())
	{
		// We aren't requesting any new zones with this operation, so don't
		// bother firing off a State Server request. Instead, let the client
		// know we're already done:

		Datagram resp;
		resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
		resp.add_uint32(context);
		resp.add_uint16(i.id);
		network_send(resp);

		return;
	}

	InterestOperation *iop = new InterestOperation(i.id, context, i.parent, new_zones);

	uint32_t request_context = m_next_context++;
	m_pending_interests.insert(std::pair<uint32_t, InterestOperation*>(request_context, iop));

	Datagram resp;
	resp.add_server_header(i.parent, m_channel, STATESERVER_OBJECT_GET_ZONES_OBJECTS);
	resp.add_uint32(request_context);
	resp.add_uint32(i.parent);
	resp.add_uint16(new_zones.size());
	for(auto it = new_zones.begin(); it != new_zones.end(); ++it)
	{
		resp.add_uint32(*it);
		subscribe_channel(LOCATION2CHANNEL(i.parent, *it));
	}
	send(resp);
}

void Client::remove_interest(Interest &i, uint32_t context)
{
	std::unordered_set<uint32_t> killed_zones;

	for(auto it = i.zones.begin(); it != i.zones.end(); ++it)
	{
		if(lookup_interests(i.parent, *it).size() == 1)
		{
			// We're the only interest who can see this zone, so let's kill it.
			killed_zones.insert(*it);
		}
	}

	// Now that we know what zones to kill, let's get to it:
	close_zones(i.parent, killed_zones);

	Datagram resp;
	resp.add_uint16(CLIENT_DONE_INTEREST_RESP);
	resp.add_uint32(context);
	resp.add_uint16(i.id);
	network_send(resp);

	m_interests.erase(i.id);
}

void Client::handle_client_object_update_field(DatagramIterator &dgi)
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
	if(data.size() > 65535u-resp.size())
	{
		send_disconnect(CLIENT_DISCONNECT_OVERSIZED_DATAGRAM, "Field update too large to be routed on MD.", true);
		return;
	}
	resp.add_data(data);
	send(resp);
}

void Client::handle_client_object_location(DatagramIterator &dgi)
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

void Client::handle_client_add_interest(DatagramIterator &dgi, bool multiple)
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

void Client::handle_client_remove_interest(DatagramIterator &dgi)
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

void Client::network_disconnect()
{
	if(!m_clean_disconnect)
	{
		std::list<std::string> event;
		event.push_back("client-lost");
		send_event(event);
	}
	delete this;
}

static ClientType<Client> client_fact("libastron");



/* ========================== *
 *       HELPER CLASSES       *
 * ========================== */
ChannelTracker::ChannelTracker(channel_t min, channel_t max) : m_next(min), m_max(max), m_unused_channels()
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

InterestOperation::InterestOperation(uint16_t interest_id, uint32_t client_context,
                                     uint32_t parent, std::unordered_set<uint32_t> zones) :
	m_interest_id(interest_id), m_client_context(client_context), m_parent(parent), m_zones(zones),
	m_has_total(false), m_total(0)
{
}

bool InterestOperation::is_ready(const std::unordered_map<uint32_t, VisibleObject> &dist_objs)
{
	if(!m_has_total)
	{
		return false;
	}

	uint32_t count = 0;
	for(auto it = dist_objs.begin(); it != dist_objs.end(); ++it)
	{
		const VisibleObject &distobj = it->second;
		if(distobj.parent == m_parent &&
		   (m_zones.find(distobj.zone) != m_zones.end()))
		{
			count++;
		}
	}

	return count >= m_total;
}

void InterestOperation::store_total(uint32_t total)
{
	if(!m_has_total)
	{
		m_total = total;
		m_has_total = true;
	}
}