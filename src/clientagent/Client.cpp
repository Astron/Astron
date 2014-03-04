#include "Client.h"
#include "ClientMessages.h"
#include "ClientAgent.h"

#include "core/global.h"
#include "core/msgtypes.h"

using dclass::Class;

Client::Client(ClientAgent* client_agent) : m_client_agent(client_agent), m_state(CLIENT_STATE_NEW),
	m_channel(0), m_allocated_channel(0), m_next_context(0), m_owned_objects(), m_seen_objects(),
	m_interests(), m_pending_interests()
{
	m_channel = m_client_agent->m_ct.alloc_channel();
	if(!m_channel)
	{
		send_disconnect(CLIENT_DISCONNECT_GENERIC, "Client capacity reached");
		return;
	}
	m_allocated_channel = m_channel;


	std::stringstream name;
	name << "Client (" << m_allocated_channel << ")";
	m_log = new LogCategory("client", name.str());
	set_con_name(name.str());

	subscribe_channel(m_channel);
	subscribe_channel(BCHAN_CLIENTS);
}

Client::~Client()
{
	m_client_agent->m_ct.free_channel(m_allocated_channel);
}

// log_event sends an event to the EventLogger
void Client::log_event(const std::list<std::string> &event)
{
	Datagram_ptr dg = Datagram::create();

	std::stringstream ss;
	ss << "Client:" << m_allocated_channel;
	dg->add_string(ss.str());

	for(auto it = event.begin(); it != event.end(); ++it)
	{
		dg->add_string(*it);
	}

	g_eventsender.send(dg);
}

// lookup_object returns the class of the object with a do_id.
// If that object is not visible to the client, NULL will be returned instead.
const Class *Client::lookup_object(doid_t do_id)
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
		if(m_visible_objects.find(do_id) != m_visible_objects.end())
		{
			return m_visible_objects[do_id].dcc;
		}
	}

	// We're at the end of our rope; we have no clue what this object is.
	return NULL;
}

// lookup_interests returns a list of all the interests that a parent-zone pair is visible to.
std::list<Interest> Client::lookup_interests(doid_t parent_id, zone_t zone_id)
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

// add_interest will start a new interest operation and retrieve all the objects an interest
// from the server, subscribing to each zone in the interest.  If the interest already
// exists, the interest will be updated with the new zones passed in by the argument.
void Client::add_interest(Interest &i, uint32_t context)
{
	std::unordered_set<zone_t> new_zones;

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
		std::unordered_set<zone_t> killed_zones;

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

		handle_interest_done(i.id, context);

		return;
	}

	InterestOperation *iop = new InterestOperation(i.id, context, i.parent, new_zones);

	uint32_t request_context = m_next_context++;
	m_pending_interests.insert(std::pair<uint32_t, InterestOperation*>(request_context, iop));

	Datagram_ptr resp = Datagram::create();
	resp->add_server_header(i.parent, m_channel, STATESERVER_OBJECT_GET_ZONES_OBJECTS);
	resp->add_uint32(request_context);
	resp->add_doid(i.parent);
	resp->add_uint16(new_zones.size());
	for(auto it = new_zones.begin(); it != new_zones.end(); ++it)
	{
		resp->add_zone(*it);
		subscribe_channel(LOCATION2CHANNEL(i.parent, *it));
	}
	route_datagram(resp);
}

// remove_interest find each zone an interest which is not part of another interest and
// passes it to close_zones() to be removed from the client's visibility.
void Client::remove_interest(Interest &i, uint32_t context)
{
	std::unordered_set<zone_t> killed_zones;

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

	handle_interest_done(i.id, context);

	m_interests.erase(i.id);
}

// cloze_zones removes objects visible through the zones from the client and unsubscribes
// from the associated location channels for those objects.
void Client::close_zones(doid_t parent, const std::unordered_set<zone_t> &killed_zones)
{
	// Kill off all objects that are in the matched parent/zones:

	std::list<doid_t> to_remove;
	for(auto it = m_visible_objects.begin(); it != m_visible_objects.end(); ++it)
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

			handle_remove_object(it->second.id);

			m_seen_objects.erase(it->second.id);
			m_historical_objects.insert(it->second.id);
			to_remove.push_back(it->second.id);
		}
	}

	for(auto it = to_remove.begin(); it != to_remove.end(); ++it)
	{
		m_visible_objects.erase(*it);
	}

	// Close all of the channels:
	for(auto it = killed_zones.begin(); it != killed_zones.end(); ++it)
	{
		unsubscribe_channel(LOCATION2CHANNEL(parent, *it));
	}
}

// is_historical_object returns true if the object was once visible to the client, but has
// since been deleted.  The return is still true even if the object has become visible again.
bool Client::is_historical_object(doid_t do_id)
{
	if(m_historical_objects.find(do_id) != m_historical_objects.end())
	{
		return true;
	}
	return false;
}

// send_disconnect must close any connections with a connected client;
// the given reason and error should be forwarded to the client.
// Client::send_disconnect can be called by subclasses to handle logging the event.
void Client::send_disconnect(uint16_t reason, const std::string &error_string, bool security)
{
	(security ? m_log->security() : m_log->error())
	        << "Ejecting client (" << reason << "): "
	        << error_string << std::endl;

	std::list<std::string> event;
	event.push_back(security ? "client-ejected-security" : "client-ejected");
	event.push_back(std::to_string((unsigned long long)reason));
	event.push_back(error_string);
	log_event(event);
}

// handle_datagram is the handler for datagrams received from the Astron cluster
void Client::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_channel();
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
			handle_drop();
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
			doid_t do_id = dgi.read_doid();
			if(!lookup_object(do_id))
			{
				m_log->warning() << "Received server-side field update for unknown object " << do_id << std::endl;
				return;
			}
			if(sender != m_channel)
			{
				uint16_t field_id = dgi.read_uint16();
				handle_set_field(do_id, field_id, dgi);
			}
		}
		break;
		case STATESERVER_OBJECT_DELETE_RAM:
		{
			doid_t do_id = dgi.read_doid();
			if(!lookup_object(do_id))
			{
				m_log->warning() << "Received server-side object delete for unknown object " << do_id << std::endl;
				return;
			}

			if(m_seen_objects.find(do_id) != m_seen_objects.end())
			{
				m_seen_objects.erase(do_id);
				m_historical_objects.insert(do_id);
				handle_remove_object(do_id);
			}

			if(m_owned_objects.find(do_id) != m_owned_objects.end())
			{
				m_owned_objects.erase(do_id);
				handle_remove_ownership(do_id);
			}

			m_visible_objects.erase(do_id);
		}
		break;
		case STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER:
        case STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED:
		{
			doid_t do_id = dgi.read_doid();
			doid_t parent = dgi.read_doid();
			zone_t zone = dgi.read_zone();
			uint16_t dc_id = dgi.read_uint16();
			m_owned_objects.insert(do_id);

			if(m_visible_objects.find(do_id) == m_visible_objects.end())
			{
				VisibleObject obj;
				obj.id = do_id;
				obj.parent = parent;
				obj.zone = zone;
				obj.dcc = g_dcf->get_class_by_id(dc_id);
				m_visible_objects[do_id] = obj;
			}

			handle_add_ownership(do_id, parent, zone, dc_id, dgi,
		                         msgtype == STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER);
		}
		break;
		case CLIENTAGENT_SET_CLIENT_ID:
		{
			if(m_channel != m_allocated_channel)
			{
				unsubscribe_channel(m_channel);
			}

			m_channel = dgi.read_channel();
			subscribe_channel(m_channel);
		}
		break;
		case CLIENTAGENT_SEND_DATAGRAM:
		{
			Datagram_ptr forward = Datagram::create();
			forward->add_data(dgi.read_string());
			forward_datagram(forward);
		}
		break;
		case CLIENTAGENT_OPEN_CHANNEL:
		{
			subscribe_channel(dgi.read_channel());
		}
		break;
		case CLIENTAGENT_CLOSE_CHANNEL:
		{
			unsubscribe_channel(dgi.read_channel());
		}
		break;
		case CLIENTAGENT_ADD_POST_REMOVE:
		{
			add_post_remove(dgi.read_datagram());
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
			doid_t do_id = dgi.read_doid();
			doid_t parent = dgi.read_doid();
			zone_t zone = dgi.read_zone();
			uint16_t dc_id = dgi.read_uint16();
			if(m_owned_objects.find(do_id) != m_owned_objects.end() ||
			        m_seen_objects.find(do_id) != m_seen_objects.end())
			{
				return;
			}
			if(m_visible_objects.find(do_id) == m_visible_objects.end())
			{
				VisibleObject obj;
				obj.id = do_id;
				obj.dcc = g_dcf->get_class_by_id(dc_id);
				obj.parent = parent;
				obj.zone = zone;
				m_visible_objects[do_id] = obj;
			}
			m_seen_objects.insert(do_id);

			handle_add_object(do_id, parent, zone, dc_id, dgi,
			                  msgtype == STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER);

			// TODO: This is a tad inefficient as it checks every pending interest.
			// In practice, there shouldn't be many add-interest operations active
			// at once, however.
			std::list<uint32_t> deferred_deletes;
			for(auto it = m_pending_interests.begin(); it != m_pending_interests.end(); ++it)
			{
				if(it->second->is_ready(m_visible_objects))
				{
					handle_interest_done(it->second->m_interest_id, it->second->m_client_context);
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
			// using doid_t because <max_objects_in_zones> == <max_total_objects>
			doid_t count = dgi.read_doid();

			if(m_pending_interests.find(context) == m_pending_interests.end())
			{
				m_log->error() << "Received GET_ZONES_COUNT_RESP for unknown context "
				               << context << std::endl;
				return;
			}

			m_pending_interests[context]->store_total(count);

			if(m_pending_interests[context]->is_ready(m_visible_objects))
			{
				handle_interest_done(m_pending_interests[context]->m_interest_id,
				                     m_pending_interests[context]->m_client_context);
				m_pending_interests.erase(context);
			}
		}
		break;
		case STATESERVER_OBJECT_CHANGING_LOCATION:
		{
			doid_t do_id = dgi.read_doid();
			doid_t n_parent = dgi.read_doid();
			zone_t n_zone = dgi.read_zone();
			dgi.skip(sizeof(doid_t) + sizeof(zone_t)); // don't care about the old location
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

			if(m_visible_objects.find(do_id) != m_visible_objects.end())
			{
				m_visible_objects[do_id].parent = n_parent;
				m_visible_objects[do_id].zone = n_zone;
			}

			if(disable && m_owned_objects.find(do_id) == m_owned_objects.end())
			{
				handle_remove_object(do_id);
				m_seen_objects.erase(do_id);
				m_historical_objects.insert(do_id);
				m_visible_objects.erase(do_id);
			}
			else
			{
				handle_change_location(do_id, n_parent, n_zone);
			}
		}
		break;
		default:
			m_log->error() << "Recv'd unk server msgtype " << msgtype << std::endl;
	}
}

/* ========================== *
 *       HELPER CLASSES       *
 * ========================== */
InterestOperation::InterestOperation(uint16_t interest_id, uint32_t client_context,
                                     doid_t parent, std::unordered_set<zone_t> zones) :
	m_interest_id(interest_id), m_client_context(client_context), m_parent(parent), m_zones(zones),
	m_has_total(false), m_total(0)
{
}

bool InterestOperation::is_ready(const std::unordered_map<doid_t, VisibleObject> &visible_objects)
{
	if(!m_has_total)
	{
		return false;
	}

	uint32_t count = 0;
	for(auto it = visible_objects.begin(); it != visible_objects.end(); ++it)
	{
		const VisibleObject &obj = it->second;
		if(obj.parent == m_parent &&
		        (m_zones.find(obj.zone) != m_zones.end()))
		{
			count++;
		}
	}

	return count >= m_total;
}

void InterestOperation::store_total(doid_t total)
{
	if(!m_has_total)
	{
		m_total = total;
		m_has_total = true;
	}
}
