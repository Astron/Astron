#include "core/global.h"
#include "core/messages.h"
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"
#include "dcparser/dcAtomicField.h"
#include "dcparser/dcMolecularField.h"

#include "DistributedObject.h"

DistributedObject::DistributedObject(StateServer *stateserver, uint32_t do_id, DCClass *dclass,
                                     uint32_t parent_id, uint32_t zone_id, DatagramIterator &dgi, bool has_other) :
	m_stateserver(stateserver), m_do_id(do_id), m_dclass(dclass), m_zone_id(zone_id),
	m_ai_channel(0), m_owner_channel(0), m_ai_explicitly_set(false)
{
	std::stringstream name;
	name << dclass->get_name() << "(" << do_id << ")";
	m_log = new LogCategory("object", name.str());

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

	m_parent_id = 0;
	handle_parent_change(parent_id);
	send_zone_entry(LOCATION2CHANNEL(m_parent_id, m_zone_id));
}

DistributedObject::~DistributedObject()
{
	delete m_log;
}

void DistributedObject::append_required_data(Datagram &dg, bool broadcast_only)
{
	dg.add_uint32(m_parent_id);
	dg.add_uint32(m_zone_id);
	dg.add_uint16(m_dclass->get_number());
	dg.add_uint32(m_do_id);
	uint32_t field_count = m_dclass->get_num_inherited_fields();
	for(uint32_t i = 0; i < field_count; ++i)
	{
		DCField *field = m_dclass->get_inherited_field(i);
		if(field->is_required() && !field->as_molecular_field() && (!broadcast_only || field->is_broadcast()))
		{
			dg.add_data(m_required_fields[field]);
		}
	}
}

void DistributedObject::append_other_data(Datagram &dg)
{
	dg.add_uint16(m_ram_fields.size());
	for(auto it = m_ram_fields.begin(); it != m_ram_fields.end(); ++it)
	{
		dg.add_uint16(it->first->get_number());
		dg.add_data(it->second);
	}
}

void DistributedObject::send_zone_entry(channel_t destination)
{
	Datagram dg(destination, m_do_id,
	            m_ram_fields.size() ?
	            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER :
	            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
	append_required_data(dg, true);
	if(m_ram_fields.size())
	{
		append_other_data(dg);
	}
	send(dg);
}

void DistributedObject::handle_parent_change(uint32_t new_parent)
{
	if(new_parent == m_parent_id)
	{
		return;    // Not actually changing parent, no need to handle.
	}

	// Unsubscribe from the old parent's child-broadcast channel.
	if(m_parent_id)
	{
		MessageDirector::singleton.unsubscribe_channel(this, LOCATION2CHANNEL(4030, m_parent_id));
	}

	m_parent_id = new_parent;

	// Subscribe to new one...
	MessageDirector::singleton.subscribe_channel(this, LOCATION2CHANNEL(4030, m_parent_id));

	if(!m_ai_explicitly_set)
	{
		// Ask the new parent what its AI is.
		Datagram dg(m_parent_id, m_do_id, STATESERVER_OBJECT_QUERY_MANAGING_AI);
		send(dg);
	}
}

void DistributedObject::handle_ai_change(channel_t new_channel, bool channel_is_explicit)
{
	if(new_channel == m_ai_channel)
	{
		return;
	}

	if(m_ai_channel)
	{
		Datagram dg(m_ai_channel, m_do_id, STATESERVER_OBJECT_LEAVING_AI_INTEREST);
		dg.add_uint32(m_do_id);
		m_log->spam() << "Leaving AI interest" << std::endl;
		send(dg);
	}

	m_ai_channel = new_channel;
	m_ai_explicitly_set = channel_is_explicit;

	Datagram dg1(m_ai_channel, m_do_id, STATESERVER_OBJECT_ENTER_AI_RECV);
	append_required_data(dg1, false);
	append_other_data(dg1);
	send(dg1);
	m_log->spam() << "Sending STATESERVER_OBJECT_ENTER_AI_RECV to "
	              << m_ai_channel << std::endl;

	Datagram dg2(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
	dg2.add_uint32(m_do_id);
	dg2.add_uint64(m_ai_channel);
	send(dg2);
}

void DistributedObject::annihilate()
{
	std::set<channel_t> targets;
	targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
	if(m_owner_channel)
	{
		targets.insert(m_owner_channel);
	}
	Datagram dg(targets, m_do_id, STATESERVER_OBJECT_DELETE_RAM);
	dg.add_uint32(m_do_id);
	send(dg);

	handle_ai_change(0, true); // Leave the AI's interest.

	m_stateserver->m_objs[m_do_id] = NULL;
	m_log->debug() << "Deleted." << std::endl;
	delete this;
}

void DistributedObject::handle_shard_reset()
{
	// Tell my children:
	Datagram dg(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_SHARD_RESET);
	dg.add_uint64(m_ai_channel);
	send(dg);
	// Fall over dead:
	annihilate();
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
		Datagram dg(targets, sender, STATESERVER_OBJECT_UPDATE_FIELD);
		dg.add_uint32(m_do_id);
		dg.add_uint16(field_id);
		dg.add_data(data);
		send(dg);
	}
	return true;
}

bool DistributedObject::handle_query(Datagram &out, uint16_t field_id)
{
	DCField *field = m_dclass->get_field_by_index(field_id);
	if(!field)
	{
		m_log->error() << "Received query for missing field ID="
		               << field_id << std::endl;
		return false;
	}

	DCMolecularField *molecular = field->as_molecular_field();
	if(molecular)
	{
		int n = molecular->get_num_atomics();
		for(int i = 0; i < n; ++i)
		{
			if(!handle_query(out, molecular->get_atomic(i)->get_number()))
			{
				return false;
			}
		}
		return true;
	}

	if(m_required_fields.count(field))
	{
		out.add_data(m_required_fields[field]);
	}
	else if(m_ram_fields.count(field))
	{
		out.add_data(m_ram_fields[field]);
	}
	else
	{
		return false;
	}

	return true;
}

void DistributedObject::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case STATESERVER_SHARD_RESET:
		{
			if(m_ai_channel != dgi.read_uint64())
			{
				m_log->warning() << " received reset for wrong AI channel" << std::endl;
				break; // Not my AI!
			}
			handle_shard_reset();

			break;
		}
		case STATESERVER_OBJECT_DELETE_RAM:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}
			annihilate();

			break;
		}
		case STATESERVER_OBJECT_UPDATE_FIELD:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}
			handle_one_update(dgi, sender);

			break;
		}
		case STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE:
		{
			if(m_do_id != dgi.read_uint32())
			{
				break;    // Not meant for me!
			}
			uint16_t field_count = dgi.read_uint16();
			for(int16_t i = 0; i < field_count; ++i)
				if(!handle_one_update(dgi, sender))
				{
					break;
				}

			break;
		}
		case STATESERVER_OBJECT_NOTIFY_MANAGING_AI:
		{
			uint32_t r_parent_id = dgi.read_uint32();
			channel_t r_ai_channel = dgi.read_uint64();
			m_log->spam() << "STATESERVER_OBJECT_NOTIFY_MANAGING_AI from " << r_parent_id << std::endl;
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
			handle_ai_change(r_ai_channel, false);

			break;
		}
		case STATESERVER_OBJECT_SET_AI_CHANNEL:
		{
			uint32_t r_do_id = dgi.read_uint32();
			channel_t r_ai_channel = dgi.read_uint64();
			m_log->spam() << "STATESERVER_OBJECT_SET_AI_CHANNEL: ai_channel=" << r_ai_channel << std::endl;
			if(r_do_id != m_do_id)
			{
				break;
			}
			handle_ai_change(r_ai_channel, true);

			break;
		}
		case STATESERVER_OBJECT_QUERY_MANAGING_AI:
		{
			m_log->spam() << "STATESERVER_OBJECT_QUERY_MANAGING_AI" << std::endl;
			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
			dg.add_uint32(m_do_id);
			dg.add_uint64(m_ai_channel);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_SET_ZONE:
		{
			uint32_t old_parent_id = m_parent_id, old_zone_id = m_zone_id;
			uint32_t new_parent_id = dgi.read_uint32();
			m_zone_id = dgi.read_uint32();

			std::set <channel_t> targets;
			targets.insert(LOCATION2CHANNEL(old_parent_id, old_zone_id));
			if(m_ai_channel)
			{
				targets.insert(m_ai_channel);
			}
			Datagram dg(targets, sender, STATESERVER_OBJECT_CHANGE_ZONE);
			dg.add_uint32(m_do_id);
			dg.add_uint32(new_parent_id);
			dg.add_uint32(m_zone_id);
			dg.add_uint32(old_parent_id);
			dg.add_uint32(old_zone_id);
			send(dg);

			handle_parent_change(new_parent_id);
			send_zone_entry(LOCATION2CHANNEL(m_parent_id, m_zone_id));

			break;
		}
		case STATESERVER_OBJECT_LOCATE:
		{
			uint32_t context = dgi.read_uint32();

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_LOCATE_RESP);
			dg.add_uint32(context);
			dg.add_uint32(m_do_id);
			dg.add_uint32(m_parent_id);
			dg.add_uint32(m_zone_id);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_QUERY_ALL:
		{
			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_ALL_RESP);
			dg.add_uint32(dgi.read_uint32()); // Copy context to response.
			append_required_data(dg, false);
			append_other_data(dg);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_QUERY_FIELD:
		{
			if(dgi.read_uint32() != m_do_id)
			{
				return;    // Not meant for this object!
			}
			uint16_t field_id = dgi.read_uint16();
			uint32_t context = dgi.read_uint32();

			Datagram raw_field;
			bool success = handle_query(raw_field, field_id);

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_FIELD_RESP);
			dg.add_uint32(m_do_id);
			dg.add_uint16(field_id);
			dg.add_uint32(context);
			dg.add_uint8(success);
			if(success)
			{
				dg.add_datagram(raw_field);
			}
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_QUERY_FIELDS:
		{
			if(dgi.read_uint32() != m_do_id)
			{
				return;    // Not meant for this object!
			}
			uint32_t context = dgi.read_uint32();

			Datagram raw_fields;
			bool success = true;
			while(dgi.tell() != in_dg.size())
			{
				uint16_t field_id = dgi.read_uint16();
				raw_fields.add_uint16(field_id);
				if(!handle_query(raw_fields, field_id))
				{
					success = false;
					break;
				}
			}

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_FIELDS_RESP);
			dg.add_uint32(m_do_id);
			dg.add_uint32(context);
			dg.add_uint8(success);
			if(success)
			{
				dg.add_datagram(raw_fields);
			}
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_SET_OWNER_RECV:
		{
			channel_t owner_channel = dgi.read_uint64();

			if(owner_channel == m_owner_channel)
			{
				return;
			}

			if(m_owner_channel)
			{
				Datagram dg(m_owner_channel, sender, STATESERVER_OBJECT_CHANGE_OWNER_RECV);
				dg.add_uint32(m_do_id);
				dg.add_uint64(owner_channel);
				dg.add_uint64(m_owner_channel);
				send(dg);
			}

			m_owner_channel = owner_channel;

			Datagram dg1(m_owner_channel, m_do_id, STATESERVER_OBJECT_ENTER_OWNER_RECV);
			append_required_data(dg1, false);
			append_other_data(dg1);
			send(dg1);

			break;
		}
		case STATESERVER_OBJECT_QUERY_ZONE_ALL:
		{
			uint32_t queried_parent = dgi.read_uint32();

			if(queried_parent == m_do_id)
			{
				std::vector<uint8_t> queried_zones = dgi.read_remainder();

				// Bounce the message down to all children and have them decide
				// whether or not to reply.
				// TODO: Is this really that efficient?
				Datagram dg(LOCATION2CHANNEL(4030, m_do_id), sender,
				            STATESERVER_OBJECT_QUERY_ZONE_ALL);
				dg.add_uint32(queried_parent);
				dg.add_data(queried_zones);
				send(dg);

				// FIXME: This assumes all objects process and reply before this goes
				// out. This will typically be the case if everything is on one State
				// Server; however, a better method should probably be used in the future.
				Datagram done_dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_ZONE_ALL_DONE);
				done_dg.add_uint32(queried_parent);
				done_dg.add_data(queried_zones);
				send(done_dg);

				break;
			}

			if(queried_parent == m_parent_id)
			{
				// Query was relayed from parent! See if we match any of the zones
				// and if so, reply:
				uint16_t zones = dgi.read_uint16();
				for(uint16_t i = 0; i < zones; ++i)
				{
					if(dgi.read_uint32() == m_zone_id)
					{
						send_zone_entry(sender);
						break;
					}
				}
			}

			break;
		}
		default:
			m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
	}
}
