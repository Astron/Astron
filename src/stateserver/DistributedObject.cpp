#include "core/global.h"
#include "core/messages.h"
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"
#include "dcparser/dcAtomicField.h"
#include "dcparser/dcMolecularField.h"

#include "DistributedObject.h"

extern std::map<unsigned int, DistributedObject*> distObjs;

static void UnpackFieldFromDG(DCPackerInterface *field, DatagramIterator &dgi, std::string &str)
{
	if(field->has_fixed_byte_size())
	{
		str += dgi.read_data(field->get_fixed_byte_size());
	}
	else if(field->get_num_length_bytes() > 0)
	{
		unsigned int length = field->get_num_length_bytes();
		switch(length)
		{
		case 2:
		{
			unsigned short l = dgi.read_uint16();
			str += std::string((char*)&l, 2);
			length = l;
		}
		break;
		case 4:
		{
			unsigned int l = dgi.read_uint32();
			str += std::string((char*)&l, 4);
			length = l;
		}
		break;
		break;
		}
		str += dgi.read_data(length);
	}
	else
	{
		unsigned int nNested = field->get_num_nested_fields();
		for(unsigned int i = 0; i < nNested; ++i)
		{
			UnpackFieldFromDG(field->get_nested_field(i), dgi, str);
		}
	}
}

DistributedObject::DistributedObject(StateServer *stateserver, unsigned int do_id, DCClass *dclass, unsigned int parent_id, unsigned int zone_id, DatagramIterator &dgi, bool has_other) :
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
			UnpackFieldFromDG(field, dgi, m_required_fields[field]);
		}
	}

	if(has_other)
	{
		unsigned short count = dgi.read_uint16();
		for(int i = 0; i < count; ++i)
		{
			unsigned int field_id = dgi.read_uint16();
			DCField *field = m_dclass->get_field_by_index(field_id);
			if(field->is_ram())
			{
				UnpackFieldFromDG(field, dgi, m_ram_fields[field]);
			}
			else
			{
				m_log->error() << "Received non-RAM field "
								<< field->get_name()
								<< " within an OTHER section" << std::endl;
			}
		}
	}

	MessageDirector::singleton.subscribe_channel(this, do_id);

	m_parent_id = 0;
	handle_parent_change(parent_id);
	send_zone_entry();
}

void DistributedObject::append_required_data(Datagram &dg)
{
	dg.add_uint32(m_parent_id);
	dg.add_uint32(m_zone_id);
	dg.add_uint16(m_dclass->get_number());
	dg.add_uint32(m_do_id);
	unsigned int field_count = m_dclass->get_num_inherited_fields();
	for(unsigned int i = 0; i < field_count; ++i)
	{
		DCField *field = m_dclass->get_inherited_field(i);
		if(field->is_required() && !field->as_molecular_field())
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

void DistributedObject::send_zone_entry()
{
	Datagram dg(LOCATION2CHANNEL(m_parent_id, m_zone_id), m_do_id,
				m_ram_fields.size() ?
				STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER :
				STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
	append_required_data(dg);
	if(m_ram_fields.size())
		append_other_data(dg);
	send(dg);
}

void DistributedObject::handle_parent_change(channel_t new_parent)
{
	if(new_parent == m_parent_id)
		return; // Not actually changing parent, no need to handle.

	// Unsubscribe from the old parent's child-broadcast channel.
	if(m_parent_id)
		MessageDirector::singleton.unsubscribe_channel(this, LOCATION2CHANNEL(4030, m_parent_id));

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
		return;

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
	append_required_data(dg1);
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
	channel_t loc = LOCATION2CHANNEL(m_parent_id, m_zone_id);
	Datagram dg(loc, m_do_id, STATESERVER_OBJECT_DELETE_RAM);
	dg.add_uint32(m_do_id);
	send(dg);
	distObjs[m_do_id] = NULL;
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

void DistributedObject::save_field(DCField *field, const std::string &data)
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
	unsigned int field_id = dgi.read_uint16();
	std::string data;
	DCField *field = m_dclass->get_field_by_index(field_id);
	if(!field)
	{
		m_log->error() << "Received update for missing field ID="
						<< field_id << std::endl;
		return false;
	}

	unsigned int field_start = dgi.tell();

	try
	{
		UnpackFieldFromDG(field, dgi, data);
	}
	catch(std::exception &e)
	{
		m_log->error() << "Received truncated update for "
						<< field->get_name() << std::endl;
		return false;
	}

	DCMolecularField *molecular = field->as_molecular_field();
	if(molecular) {
		dgi.seek(field_start);
		int n = molecular->get_num_atomics();
		for(int i = 0; i < n; ++i)
		{
			std::string atomic_data;
			DCAtomicField *atomic = molecular->get_atomic(i);
			UnpackFieldFromDG(atomic, dgi, atomic_data);
			save_field(atomic->as_field(), atomic_data);
		}
	}
	else
	{
		save_field(field, data);
	}

	std::set <channel_t> targets;
	if(field->is_broadcast())
		targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
	if(field->is_airecv() && m_ai_channel)
		targets.insert(m_ai_channel);
	if(field->is_ownrecv() && m_owner_channel)
		targets.insert(m_owner_channel);
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

bool DistributedObject::handle_query(Datagram &out, unsigned short field_id)
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
				return false;
		}
		return true;
	}

	if(m_required_fields.count(field))
		out.add_data(m_required_fields[field]);
	else if(m_ram_fields.count(field))
		out.add_data(m_ram_fields[field]);
	else
		return false;

	return true;
}

void DistributedObject::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	unsigned short msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case STATESERVER_SHARD_RESET:
		{
			if(m_ai_channel != dgi.read_uint64()) {
				m_log->warning() << " received reset for wrong"
									<< " AI channel" << std::endl;
				break; // Not my AI!
			}
			handle_shard_reset();
			break;
		}
		case STATESERVER_OBJECT_DELETE_RAM:
		{
			if(m_do_id != dgi.read_uint32())
				break; // Not meant for me!
			annihilate();
			break;
		}
		case STATESERVER_OBJECT_UPDATE_FIELD:
		{
			if(m_do_id != dgi.read_uint32())
				break; // Not meant for me!
			handle_one_update(dgi, sender);
			break;
		}
		case STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE:
		{
			if(m_do_id != dgi.read_uint32())
				break; // Not meant for me!
			int field_count = dgi.read_uint16();
			for(int i = 0; i < field_count; ++i)
				if(!handle_one_update(dgi, sender))
					break;
			break;
		}
		case STATESERVER_OBJECT_NOTIFY_MANAGING_AI:
		{
			unsigned int r_parent_id = dgi.read_uint32();
			channel_t r_ai_channel = dgi.read_uint64();
			m_log->spam() << "STATESERVER_OBJECT_NOTIFY_MANAGING_AI from " << r_parent_id << std::endl;
			if(r_parent_id != m_parent_id)
			{
				m_log->warning() << "Received AI channel from " << r_parent_id
									<< " but my parent_id is " << m_parent_id << std::endl;
				break;
			}
			if(m_ai_explicitly_set)
				break;
			handle_ai_change(r_ai_channel, false);
			break;
		}
		case STATESERVER_OBJECT_SET_AI_CHANNEL:
		{
			unsigned int r_do_id = dgi.read_uint32();
			channel_t r_ai_channel = dgi.read_uint64();
			m_log->spam() << "STATESERVER_OBJECT_SET_AI_CHANNEL: ai_channel=" << r_ai_channel << std::endl;
			if(r_do_id != m_do_id)
				break;
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
			unsigned int old_parent_id = m_parent_id, old_zone_id = m_zone_id;
			unsigned int new_parent_id = dgi.read_uint32();
			m_zone_id = dgi.read_uint32();

			std::set <channel_t> targets;
			targets.insert(LOCATION2CHANNEL(old_parent_id, old_zone_id));
			if(m_ai_channel)
				targets.insert(m_ai_channel);
			Datagram dg(targets, sender, STATESERVER_OBJECT_CHANGE_ZONE);
			dg.add_uint32(m_do_id);
			dg.add_uint32(new_parent_id);
			dg.add_uint32(m_zone_id);
			dg.add_uint32(old_parent_id);
			dg.add_uint32(old_zone_id);
			send(dg);

			handle_parent_change(new_parent_id);
			send_zone_entry();
			break;
		}
		case STATESERVER_OBJECT_LOCATE:
		{
			unsigned int context = dgi.read_uint32();

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
			append_required_data(dg);
			append_other_data(dg);
			send(dg);
			break;
		}
		case STATESERVER_OBJECT_QUERY_FIELD:
		{
			if(dgi.read_uint32() != m_do_id)
				return; // Not meant for this object!
			unsigned short field_id = dgi.read_uint16();
			unsigned int context = dgi.read_uint32();

			Datagram raw_field;
			bool success = handle_query(raw_field, field_id);

			Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_FIELD_RESP);
			dg.add_uint32(m_do_id);
			dg.add_uint16(field_id);
			dg.add_uint32(context);
			dg.add_uint8(success);
			if(success)
				dg.add_datagram(raw_field);
			send(dg);
			break;
		}
		case STATESERVER_OBJECT_QUERY_FIELDS:
		{
			if(dgi.read_uint32() != m_do_id)
				return; // Not meant for this object!
			unsigned int context = dgi.read_uint32();

			Datagram raw_fields;
			bool success = true;
			while(dgi.tell() != in_dg.get_buf_end())
			{
				unsigned short field_id = dgi.read_uint16();
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
				dg.add_datagram(raw_fields);
			send(dg);
			break;
		}
		case STATESERVER_OBJECT_SET_OWNER_RECV:
		{
			channel_t owner_channel = dgi.read_uint64();

			if(owner_channel == m_owner_channel)
				return;

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
			append_required_data(dg1);
			append_other_data(dg1);
			send(dg1);

			break;
		}
		default:
			m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
	}
}
