#include "core/global.h"
#include "core/messages.h"
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"
#include "dcparser/dcAtomicField.h"
#include "dcparser/dcMolecularField.h"

#include "DistributedObject.h"

DistributedObject::DistributedObject(StateServer *stateserver, uint32_t do_id, uint32_t parent_id,
                                     uint32_t zone_id, DCClass *dclass, DatagramIterator &dgi,
                                     bool has_other) :
	m_stateserver(stateserver), m_do_id(do_id), m_parent_id(INVALID_DO_ID), m_zone_id(INVALID_ZONE),
	m_dclass(dclass), m_ai_channel(INVALID_CHANNEL), m_owner_channel(INVALID_CHANNEL),
	m_ai_explicitly_set(false), m_next_context(0), m_child_count(0)
{
	std::stringstream name;
	name << dclass->get_name() << "(" << do_id << ")";
	m_log = new LogCategory("object", name.str());
	set_con_name(name.str());

	for(int i = 0; i < m_dclass->get_num_inherited_fields(); ++i)
	{
		DCField *field = m_dclass->get_inherited_field(i);
		if(field->is_required() && !field->as_molecular_field())
		{
			dgi.unpack_field(field, m_required_fields[field]);
		}
	}

	if(has_other)
	{
		uint16_t count = dgi.read_uint16();
		for(int i = 0; i < count; ++i)
		{
			uint32_t field_id = dgi.read_uint16();
			DCField *field = m_dclass->get_field_by_index(field_id);
			if(field->is_ram())
			{
				dgi.unpack_field(field, m_ram_fields[field]);
			}
			else
			{
				m_log->error() << "Received non-RAM field " << field->get_name()
				               << " within an OTHER section" << std::endl;
			}
		}
	}

	MessageDirector::singleton.subscribe_channel(this, do_id);

	m_log->debug() << "Object created..." << std::endl;

	dgi.seek_payload(); // Seek back to front of payload, to read sender
	handle_location_change(parent_id, zone_id, dgi.read_uint64());
}

DistributedObject::DistributedObject(StateServer *stateserver, uint64_t sender, uint32_t do_id,
	                                 uint32_t parent_id, uint32_t zone_id, DCClass *dclass,
		                             std::unordered_map<DCField*, std::vector<uint8_t> > required,
		                             std::map<DCField*, std::vector<uint8_t> > ram) :
	m_stateserver(stateserver), m_do_id(do_id), m_parent_id(INVALID_DO_ID), m_zone_id(INVALID_ZONE),
	m_dclass(dclass), m_ai_channel(INVALID_CHANNEL), m_owner_channel(INVALID_CHANNEL),
	m_ai_explicitly_set(false), m_next_context(0), m_child_count(0)
{
	std::stringstream name;
	name << dclass->get_name() << "(" << do_id << ")";
	m_log = new LogCategory("object", name.str());

	m_required_fields = required;
	m_ram_fields = ram;

	MessageDirector::singleton.subscribe_channel(this, do_id);
	handle_location_change(parent_id, zone_id, sender);
}

DistributedObject::~DistributedObject()
{
	delete m_log;
}

void DistributedObject::append_required_data(Datagram &dg, bool broadcast_only, bool also_owner)
{
	dg.add_uint32(m_do_id);
	dg.add_uint32(m_parent_id);
	dg.add_uint32(m_zone_id);
	dg.add_uint16(m_dclass->get_number());
	uint32_t field_count = m_dclass->get_num_inherited_fields();
	for(uint32_t i = 0; i < field_count; ++i)
	{
		DCField *field = m_dclass->get_inherited_field(i);
		if(field->is_required() && !field->as_molecular_field() && (!broadcast_only
		        || field->is_broadcast() || (also_owner && field->is_ownrecv())))
		{
			dg.add_data(m_required_fields[field]);
		}
	}
}

void DistributedObject::append_other_data(Datagram &dg, bool broadcast_only, bool also_owner)
{
	if(broadcast_only)
	{
		std::list<DCField*> broadcast_fields;
		for(auto it = m_ram_fields.begin(); it != m_ram_fields.end(); ++it)
		{
			if(it->first->is_broadcast() || (also_owner && it->first->is_ownrecv()))
			{
				broadcast_fields.push_back(it->first);
			}
		}

		dg.add_uint16(broadcast_fields.size());
		for(auto it = broadcast_fields.begin(); it != broadcast_fields.end(); ++it)
		{
			dg.add_uint16((*it)->get_number());
			dg.add_data(m_ram_fields[*it]);
		}
	}
	else
	{
		dg.add_uint16(m_ram_fields.size());
		for(auto it = m_ram_fields.begin(); it != m_ram_fields.end(); ++it)
		{
			dg.add_uint16(it->first->get_number());
			dg.add_data(it->second);
		}
	}
}



void DistributedObject::send_location_entry(channel_t location)
{
	Datagram dg(location, m_do_id, m_ram_fields.size() ?
	            STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER :
	            STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED);
	append_required_data(dg, true);
	if(m_ram_fields.size())
	{
		append_other_data(dg, true);
	}
	send(dg);
}

void DistributedObject::send_ai_entry(channel_t ai)
{
	Datagram dg(ai, m_do_id, m_ram_fields.size() ?
	            STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER :
	            STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED);
	append_required_data(dg);
	if(m_ram_fields.size())
	{
		append_other_data(dg);
	}
	send(dg);
}

void DistributedObject::send_owner_entry(channel_t owner)
{
	Datagram dg(owner, m_do_id, m_ram_fields.size() ?
	            STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER :
	            STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED);
	append_required_data(dg, true, true);
	if(m_ram_fields.size())
	{
		append_other_data(dg, true, true);
	}
	send(dg);
}

void DistributedObject::handle_location_change(uint32_t new_parent, uint32_t new_zone,
        channel_t sender)
{
	uint32_t old_parent = m_parent_id;
	uint32_t old_zone = m_zone_id;

	// Set of channels that must be notified about location_change
	std::set<channel_t> targets;

	// Notify AI of changing location
	if(m_ai_channel)
	{
		targets.insert(m_ai_channel);
	}

	// Notify Owner of changing location
	if(m_owner_channel)
	{
		targets.insert(m_owner_channel);
	}

	if(new_parent == m_do_id)
	{
		m_log->warning() << "Object cannot be parented to itself." << std::endl;
		return;
	}

	// Handle parent change
	if(new_parent != old_parent)
	{
		// Unsubscribe from the old parent's child-broadcast channel.
		if(old_parent) // If we have an old parent
		{
			MessageDirector::singleton.unsubscribe_channel(this, PARENT2CHILDREN(m_parent_id));
			// Notify old parent of changing location
			targets.insert(old_parent);
			// Notify old location of changing location
			targets.insert(LOCATION2CHANNEL(old_parent, old_zone));
		}

		m_parent_id = new_parent;
		m_zone_id = new_zone;

		// Subscribe to new one...
		if(new_parent) // If we have a new parent
		{
			MessageDirector::singleton.subscribe_channel(this, PARENT2CHILDREN(m_parent_id));
			if(!m_ai_explicitly_set)
			{
				// Ask the new parent what its AI is.
				Datagram dg(m_parent_id, m_do_id, STATESERVER_OBJECT_GET_AI);
				dg.add_uint32(m_next_context++);
				send(dg);
			}
			targets.insert(new_parent); // Notify new parent of changing location
		}
		else if(!m_ai_explicitly_set)
		{
			m_ai_channel = 0;
		}
	}
	else if(new_zone != old_zone)
	{
		m_zone_id = new_zone;
		// Notify parent of changing zone
		targets.insert(m_parent_id);
		// Notify old location of changing location
		targets.insert(LOCATION2CHANNEL(m_parent_id, old_zone));
	}
	else
	{
		return; // Not actually changing location, no need to handle.
	}

	// Send changing location message
	Datagram dg(targets, sender, STATESERVER_OBJECT_CHANGING_LOCATION);
	dg.add_uint32(m_do_id);
	dg.add_uint32(new_parent);
	dg.add_uint32(new_zone);
	dg.add_uint32(old_parent);
	dg.add_uint32(old_zone);
	send(dg);

	// Send enter location message
	if(new_parent)
	{
		send_location_entry(LOCATION2CHANNEL(new_parent, new_zone));
	}
}

void DistributedObject::handle_ai_change(channel_t new_ai, channel_t sender,
        bool channel_is_explicit)
{
	uint64_t old_ai = m_ai_channel;
	if(new_ai == old_ai)
	{
		return;
	}

	// Set of channels that must be notified about ai_change
	std::set<channel_t> targets;

	if(old_ai)
	{
		targets.insert(old_ai);
	}
	if(m_child_count)
	{
		targets.insert(PARENT2CHILDREN(m_do_id));
	}

	m_ai_channel = new_ai;
	m_ai_explicitly_set = channel_is_explicit;

	Datagram dg(targets, sender, STATESERVER_OBJECT_CHANGING_AI);
	dg.add_uint32(m_do_id);
	dg.add_uint64(new_ai);
	dg.add_uint64(old_ai);
	send(dg);

	if(new_ai)
	{
		m_log->spam() << "Sending AI entry to "
		              << new_ai << std::endl;
		send_ai_entry(new_ai);
	}

}

void DistributedObject::annihilate(channel_t sender, bool notify_parent)
{
	std::set<channel_t> targets;
	if(m_parent_id)
	{
		targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
		// Leave parent on explicit delete ram
		if(notify_parent)
		{
			Datagram dg(m_parent_id, sender, STATESERVER_OBJECT_CHANGING_LOCATION);
			dg.add_uint32(m_do_id);
			dg.add_uint32(INVALID_DO_ID);
			dg.add_uint32(INVALID_ZONE);
			dg.add_uint32(m_parent_id);
			dg.add_uint32(m_zone_id);
			send(dg);
		}
	}
	if(m_owner_channel)
	{
		targets.insert(m_owner_channel);
	}
	if(m_ai_channel)
	{
		targets.insert(m_ai_channel);
	}
	Datagram dg(targets, sender, STATESERVER_OBJECT_DELETE_RAM);
	dg.add_uint32(m_do_id);
	send(dg);

	delete_children(sender);

	m_stateserver->m_objs.erase(m_do_id);
	m_log->debug() << "Deleted." << std::endl;
	delete this;
}

void DistributedObject::delete_children(channel_t sender)
{
	if(m_child_count)
	{
		Datagram dg(PARENT2CHILDREN(m_do_id), sender,
	                STATESERVER_OBJECT_DELETE_CHILDREN);
		dg.add_uint32(m_do_id);
		send(dg);
	}
}

void DistributedObject::save_field(DCField *field, const std::vector<uint8_t> &data)
{
	if(field->is_required())
	{
		m_required_fields[field] = data;
	}
	else if(field->is_ram())
	{
		m_ram_fields[field] = data;
	}
}

bool DistributedObject::handle_one_update(DatagramIterator &dgi, channel_t sender)
{
	std::vector<uint8_t> data;
	uint32_t field_id = dgi.read_uint16();
	DCField *field = m_dclass->get_field_by_index(field_id);
	if(!field)
	{
		m_log->error() << "Received update for missing field ID="
		               << field_id << std::endl;
		return false;
	}

	m_log->spam() << "Handling update for '" << field->get_name() << "'." << std::endl;

	uint32_t field_start = dgi.tell();

	try
	{
		dgi.unpack_field(field, data);
	}
	catch(std::exception &e)
	{
		m_log->error() << "Received truncated update for "
		               << field->get_name() << std::endl;
		return false;
	}

	DCMolecularField *molecular = field->as_molecular_field();
	if(molecular)
	{
		dgi.seek(field_start);
		int n = molecular->get_num_atomics();
		for(int i = 0; i < n; ++i)
		{
			std::vector<uint8_t> atomic_data;
			DCAtomicField *atomic = molecular->get_atomic(i);
			dgi.unpack_field(atomic, atomic_data);
			save_field(atomic->as_field(), atomic_data);
		}
	}
	else
	{
		save_field(field, data);
	}

	std::set <channel_t> targets;
	if(field->is_broadcast())
	{
		targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
	}
	if(field->is_airecv() && m_ai_channel)
	{
		targets.insert(m_ai_channel);
	}
	if(field->is_ownrecv() && m_owner_channel)
	{
		targets.insert(m_owner_channel);
	}
	if(targets.size()) // TODO: Review this for efficiency?
	{
		Datagram dg(targets, sender, STATESERVER_OBJECT_SET_FIELD);
		dg.add_uint32(m_do_id);
		dg.add_uint16(field_id);
		dg.add_data(data);
		send(dg);
	}
	return true;
}

bool DistributedObject::handle_one_get(Datagram &out, uint16_t field_id,
                                       bool succeed_if_unset, bool is_subfield)
{
	DCField *field = m_dclass->get_field_by_index(field_id);
	if(!field)
	{
		m_log->error() << "Received get_field for field: " << field_id
		               << ", not valid for class: " << m_dclass->get_name() << std::endl;
		return false;
	}
	m_log->spam() << "Handling query for '" << field->get_name() << "'." << std::endl;

	DCMolecularField *molecular = field->as_molecular_field();
	if(molecular)
	{
		int n = molecular->get_num_atomics();
		out.add_uint16(field_id);
		for(int i = 0; i < n; ++i)
		{
			if(!handle_one_get(out, molecular->get_atomic(i)->get_number(), succeed_if_unset, true))
			{
				return false;
			}
		}
		return true;
	}

	if(m_required_fields.count(field))
	{
		if(!is_subfield)
		{
			out.add_uint16(field_id);
		}
		out.add_data(m_required_fields[field]);
	}
	else if(m_ram_fields.count(field))
	{
		if(!is_subfield)
		{
			out.add_uint16(field_id);
		}
		out.add_data(m_ram_fields[field]);
	}
	else
	{
		return succeed_if_unset;
	}

	return true;
}

void DistributedObject::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case STATESERVER_DELETE_AI_OBJECTS:
		{
			if(m_ai_channel != dgi.read_uint64())
			{
				m_log->warning() << " received reset for wrong AI channel" << std::endl;
				break; // Not my AI!
			}
			annihilate(sender);

			break;
		}
		case STATESERVER_OBJECT_DELETE_RAM:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}

			// Delete object
			annihilate(sender);

			break;
		}
		case STATESERVER_OBJECT_DELETE_CHILDREN:
		{
			uint32_t r_do_id = dgi.read_uint32();
			if(r_do_id == m_do_id)
			{
				delete_children(sender);
			}
			else if(r_do_id == m_parent_id)
			{
				annihilate(sender, false);
			}
			break;
		}
		case STATESERVER_OBJECT_SET_FIELD:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}
			handle_one_update(dgi, sender);

			break;
		}
		case STATESERVER_OBJECT_SET_FIELDS:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}
			uint16_t field_count = dgi.read_uint16();
			for(int16_t i = 0; i < field_count; ++i)
			{
				if(!handle_one_update(dgi, sender))
				{
					break;
				}
			}
			break;
		}
		case STATESERVER_OBJECT_CHANGING_AI:
		{
			uint32_t r_parent_id = dgi.read_uint32();
			channel_t new_channel = dgi.read_uint64();
			m_log->spam() << "Received ChangingAI notification from " << r_parent_id << std::endl;
			if(r_parent_id != m_parent_id)
			{
				m_log->warning() << "Received AI channel from " << r_parent_id
				                 << " but my parent_id is " << m_parent_id << std::endl;
				break;
			}
			if(m_ai_explicitly_set)
			{
				break;
			}
			handle_ai_change(new_channel, sender, false);

			break;
		}
		case STATESERVER_OBJECT_SET_AI:
		{
			channel_t new_channel = dgi.read_uint64();
			m_log->spam() << "Updating AI to " << new_channel << std::endl;
			handle_ai_change(new_channel, sender, true);

			break;
		}
		case STATESERVER_OBJECT_GET_AI:
		{
			m_log->spam() << "Received AI query from " << sender << std::endl;
			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_GET_AI_RESP);
			dg.add_uint32(dgi.read_uint32()); // Get context
			dg.add_uint32(m_do_id);
			dg.add_uint64(m_ai_channel);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_AI_RESP:
		{
			dgi.read_uint32(); // Discard context
			uint32_t r_parent_id = dgi.read_uint32();
			m_log->spam() << "Received AI query response from " << r_parent_id << std::endl;
			if(r_parent_id != m_parent_id)
			{
				m_log->warning() << "Received AI channel from " << r_parent_id
				                 << " but my parent_id is " << m_parent_id << std::endl;
				break;
			}

			channel_t new_ai = dgi.read_uint64();
			if(m_ai_explicitly_set)
			{
				break;
			}
			handle_ai_change(new_ai, sender, false);

			break;
		}
		case STATESERVER_OBJECT_CHANGING_LOCATION:
		{
			uint32_t child_id = dgi.read_uint32();
			uint32_t new_parent = dgi.read_uint32();
			uint32_t new_zone = dgi.read_uint32();
			uint32_t r_do_id = dgi.read_uint32();
			uint32_t r_zone = dgi.read_uint32();
			if(new_parent == m_do_id)
			{
				if(new_parent != r_do_id)
				{
					m_child_count++;
				}
				else
				{
					if(new_zone == r_zone)
					{
						break; // No change, so do nothing.
					}

					m_zone_count[r_zone] = m_zone_count[r_zone] - 1;
				}

				m_zone_count[new_zone] = m_zone_count[new_zone] + 1;
			}
			else if(r_do_id == m_do_id)
			{
				m_zone_count[r_zone] = m_zone_count[r_zone] - 1;
				m_child_count--;
			}
			else
			{
				m_log->warning() << "Received changing location from " << child_id
				                 << " for " << r_do_id << ", but my id is " << m_do_id << std::endl;
			}

			break;
		}
		case STATESERVER_OBJECT_SET_LOCATION:
		{
			uint32_t new_parent = dgi.read_uint32();
			uint32_t new_zone = dgi.read_uint32();
			m_log->spam() << "Updating location to Parent: " << new_parent
			              << ", Zone: " << new_zone << std::endl;

			handle_location_change(new_parent, new_zone, sender);

			break;
		}
		case STATESERVER_OBJECT_GET_LOCATION:
		{
			uint32_t context = dgi.read_uint32();

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_GET_LOCATION_RESP);
			dg.add_uint32(context);
			dg.add_uint32(m_do_id);
			dg.add_uint32(m_parent_id);
			dg.add_uint32(m_zone_id);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_ALL:
		{
			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_GET_ALL_RESP);
			dg.add_uint32(dgi.read_uint32()); // Copy context to response.
			append_required_data(dg);
			append_other_data(dg);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_FIELD:
		{
			uint32_t context = dgi.read_uint32();
			if(dgi.read_uint32() != m_do_id)
			{
				return;    // Not meant for this object!
			}
			uint16_t field_id = dgi.read_uint16();

			Datagram raw_field;
			bool success = handle_one_get(raw_field, field_id);

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
			dg.add_uint32(context);
			dg.add_uint8(success);
			if(success)
			{
				dg.add_datagram(raw_field);
			}
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_FIELDS:
		{
			uint32_t context = dgi.read_uint32();
			if(dgi.read_uint32() != m_do_id)
			{
				return;    // Not meant for this object!
			}
			uint16_t field_count = dgi.read_uint16();

			Datagram raw_fields;
			bool success = true;
			uint16_t fields_found = 0;
			for(int i = 0; i < field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				uint16_t length = raw_fields.size();
				if(!handle_one_get(raw_fields, field_id, true))
				{
					success = false;
					break;
				}
				if(raw_fields.size() > length)
				{
					fields_found++;
				}
			}

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_GET_FIELDS_RESP);
			dg.add_uint32(context);
			dg.add_uint8(success);
			if(success)
			{
				dg.add_uint16(fields_found);
				dg.add_datagram(raw_fields);
			}
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_SET_OWNER:
		{
			channel_t new_owner = dgi.read_uint64();
			m_log->spam() << "Updating owner to " << new_owner << "..." << std::endl;
			if(new_owner == m_owner_channel)
			{
				m_log->spam() << "... owner is the same, do nothing." << std::endl;
				return;
			}

			if(m_owner_channel)
			{
				m_log->spam() << "... broadcasting changing owner..." << std::endl;
				Datagram dg(m_owner_channel, sender, STATESERVER_OBJECT_CHANGING_OWNER);
				dg.add_uint32(m_do_id);
				dg.add_uint64(new_owner);
				dg.add_uint64(m_owner_channel);
				send(dg);
			}

			m_owner_channel = new_owner;

			if(new_owner)
			{
				m_log->spam() << "... sending owner entry..." << std::endl;
				send_owner_entry(new_owner);
			}

			m_log->spam() << "... updated owner." << std::endl;
			break;
		}
		case STATESERVER_OBJECT_GET_ZONES_OBJECTS:
		{
			uint32_t context  = dgi.read_uint32();
			uint32_t queried_parent = dgi.read_uint32();


			m_log->spam() << "Handling get_zones_objects with parent '" << queried_parent << "'"
			              << ".  My id is " << m_do_id << " and my parent is " << m_parent_id
			              << "." << std::endl;

			if(queried_parent == m_parent_id)
			{
				// Query was relayed from parent! See if we match any of the zones
				// and if so, reply:
				uint16_t zone_count = dgi.read_uint16();
				for(uint16_t i = 0; i < zone_count; ++i)
				{
					if(dgi.read_uint32() == m_zone_id)
					{
						send_location_entry(sender);
						break;
					}
				}

				break;
			}

			if(queried_parent == m_do_id)
			{
				uint32_t child_count = 0;
				uint16_t zone_count = dgi.read_uint16();

				// Start datagram to relay to children
				Datagram child_dg(PARENT2CHILDREN(m_do_id), sender,
				                  STATESERVER_OBJECT_GET_ZONES_OBJECTS);
				child_dg.add_uint32(context);
				child_dg.add_uint32(queried_parent);
				child_dg.add_uint16(zone_count);

				// Get all zones requested
				for(int i = 0; i < zone_count; ++i)
				{
					uint32_t zone = dgi.read_uint32();
					child_count += m_zone_count[zone];
					child_dg.add_uint32(zone);
				}

				// Reply to requestor with count of objects expected
				Datagram count_dg(sender, m_do_id, STATESERVER_OBJECT_GET_ZONES_COUNT_RESP);
				count_dg.add_uint32(context);
				count_dg.add_uint32(child_count);
				send(count_dg);

				// Bounce the message down to all children and have them decide
				// whether or not to reply.
				// TODO: Is this really that efficient?
				if(child_count > 0)
				{
					send(child_dg);
				}

				break;
			}

			break;
		}
		default:
			if(msgtype < STATESERVER_MSGTYPE_MIN || msgtype > STATESERVER_MSGTYPE_MAX)
			{
				m_log->warning() << "Received unknown message of type " << msgtype << std::endl;
			}
			else
			{
				m_log->spam() << "Ignoring stateserver message of type " << msgtype << std::endl;
			}
	}
}
