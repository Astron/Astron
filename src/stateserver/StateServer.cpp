#include "util/Role.h"
#include "core/RoleFactory.h"
#include "core/global.h"
#include "core/messages.h"
#include <map>
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"
#include <exception>
#include <stdexcept>


void UnpackFieldFromDG(DCPackerInterface *field, DatagramIterator &dgi, std::string &str)
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

class DistributedObject;

std::map<unsigned int, DistributedObject*> distObjs;

class DistributedObject : public MDParticipantInterface
{
public:
	channel_t m_owner;
	unsigned int m_parent_id;
	unsigned int m_zone_id;
	unsigned int m_do_id;
	DCClass *m_dclass;
	std::map<DCField*, std::string> m_ram_fields;
	std::map<DCField*, std::string> m_required_fields;
	channel_t m_ai_channel;
	bool m_ai_explicitly_set;
	LogCategory *m_log;

	DistributedObject(unsigned int do_id, DCClass *dclass, unsigned int parent_id, unsigned int zone_id, DatagramIterator &dgi, bool has_other) :
	m_do_id(do_id), m_dclass(dclass), m_zone_id(zone_id),
	m_ai_channel(0), m_ai_explicitly_set(false)
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

	void append_required_data(Datagram &dg)
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

	void append_other_data(Datagram &dg)
	{
		dg.add_uint16(m_ram_fields.size());
		for(auto it = m_ram_fields.begin(); it != m_ram_fields.end(); ++it)
		{
			dg.add_uint16(it->first->get_number());
			dg.add_data(it->second);
		}
	}

	void send_zone_entry()
	{
		Datagram dg(LOCATION2CHANNEL(m_parent_id, m_zone_id), m_do_id,
		            m_ram_fields.size() ?
		            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER :
		            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
		append_required_data(dg);
		if(m_ram_fields.size())
			append_other_data(dg);
		send(&dg);
	}

	void handle_parent_change(channel_t new_parent)
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
			send(&dg);
		}
	}

	void handle_ai_change(channel_t new_channel, bool channel_is_explicit)
	{
		if(new_channel == m_ai_channel)
			return;

		if(m_ai_channel)
		{
			Datagram dg(m_ai_channel, m_do_id, STATESERVER_OBJECT_LEAVING_AI_INTEREST);
			dg.add_uint32(m_do_id);
			m_log->spam() << "Leaving AI interest" << std::endl;
			send(&dg);
		}

		m_ai_channel = new_channel;
		m_ai_explicitly_set = channel_is_explicit;

		Datagram dg1(m_ai_channel, m_do_id, STATESERVER_OBJECT_ENTER_AI_RECV);
		append_required_data(dg1);
		append_other_data(dg1);
		send(&dg1);
		m_log->spam() << "Sending STATESERVER_OBJECT_ENTER_AI_RECV to "
		              << m_ai_channel << std::endl;

		Datagram dg2(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
		dg2.add_uint32(m_do_id);
		dg2.add_uint64(m_ai_channel);
		send(&dg2);
	}

	void annihilate()
	{
		channel_t loc = LOCATION2CHANNEL(m_parent_id, m_zone_id);
		Datagram dg(loc, m_do_id, STATESERVER_OBJECT_DELETE_RAM);
		dg.add_uint32(m_do_id);
		send(&dg);
		distObjs[m_do_id] = NULL;
		m_log->debug() << "Deleted." << std::endl;
		delete this;
	}

	void handle_shard_reset()
	{
		// Tell my children:
		Datagram dg(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_SHARD_RESET);
		dg.add_uint64(m_ai_channel);
		send(&dg);
		// Fall over dead:
		annihilate();
	}

	bool handle_one_update(DatagramIterator &dgi, channel_t sender)
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
		if(field->is_required())
		{
			m_required_fields[field] = data;
		}
		else if(field->is_ram())
		{
			m_ram_fields[field] = data;
		}

		std::set <channel_t> targets;
		if(field->is_broadcast())
			targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
		if(field->is_airecv() && m_ai_channel)
			targets.insert(m_ai_channel);
		if(targets.size()) // TODO: Review this for efficiency?
		{
			Datagram dg(targets, sender, STATESERVER_OBJECT_UPDATE_FIELD);
			dg.add_uint32(m_do_id);
			dg.add_uint16(field_id);
			dg.add_data(data);
			send(&dg);
		}
		return true;
	}

	virtual bool handle_datagram(Datagram *in_dg, DatagramIterator &dgi)
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
				send(&dg);
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
				send(&dg);

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
				send(&dg);
				break;
			}
			case STATESERVER_OBJECT_QUERY_ALL:
			{
				Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_ALL_RESP);
				dg.add_uint32(dgi.read_uint32()); // Copy context to response.
				append_required_data(dg);
				append_other_data(dg);
				send(&dg);
				break;
			}
			default:
				m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
		}
		return true;
	}
};

ConfigVariable<channel_t> cfg_channel("control", 0);

class StateServer : public Role
{
public:
	StateServer(RoleConfig roleconfig) : Role(roleconfig)
	{
		channel_t channel = cfg_channel.get_rval(m_roleconfig);
		MessageDirector::singleton.subscribe_channel(this, channel);

		std::stringstream name;
		name << "StateServer(" << channel << ")";
		m_log = new LogCategory("stateserver", name.str());
	}

	void handle_generate(DatagramIterator &dgi, bool has_other)
	{
		unsigned int parent_id = dgi.read_uint32();
		unsigned int zone_id = dgi.read_uint32();
		unsigned short dc_id = dgi.read_uint16();
		unsigned int do_id = dgi.read_uint32();

		if(dc_id >= gDCF->get_num_classes())
		{
			m_log->error() << "Received create for unknown dclass ID=" << dc_id << std::endl;
			return;
		}

		if(distObjs.find(do_id) != distObjs.end())
		{
			m_log->warning() << "Received generate for already-existing object ID=" << do_id << std::endl;
			return;
		}

		DCClass *dclass = gDCF->get_class(dc_id);
		DistributedObject *obj;
		try
		{
			obj = new DistributedObject(do_id, dclass, parent_id, zone_id, dgi, has_other);
		}
		catch(std::exception &e)
		{
			m_log->error() << "Received truncated generate for "
			               << dclass->get_name() << "(" << do_id << ")" << std::endl;
			return;
		}
		distObjs[do_id] = obj;
	}

	virtual bool handle_datagram(Datagram *in_dg, DatagramIterator &dgi)
	{
		channel_t sender = dgi.read_uint64();
		unsigned short msgtype = dgi.read_uint16();
		switch(msgtype)
		{
			case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED:
			{
				handle_generate(dgi, false);
				break;
			}
			case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER:
			{
				handle_generate(dgi, true);
				break;
			}
			case STATESERVER_SHARD_RESET:
			{
				channel_t ai_channel = dgi.read_uint64();
				std::set <channel_t> targets;
				for(auto it = distObjs.begin(); it != distObjs.end(); ++it)
					if(it->second && it->second->m_ai_channel == ai_channel && it->second->m_ai_explicitly_set)
						targets.insert(it->second->m_do_id);

				if(targets.size())
				{
					Datagram dg(targets, sender, STATESERVER_SHARD_RESET);
					dg.add_uint64(ai_channel);
					send(&dg);
				}
				break;
			}
			default:
				m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
		}
		return true;
	}

private:
	LogCategory *m_log;
};

RoleFactoryItem<StateServer> ss_fact("stateserver");
