#include "util/Role.h"
#include "core/RoleFactory.h"
#include "core/global.h"
#include <map>
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"

#pragma region definitions
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED 2001
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER 2003
#define STATESERVER_OBJECT_UPDATE_FIELD  2004
#define STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE  2005
#define STATESERVER_OBJECT_DELETE_RAM  2007
#define STATESERVER_OBJECT_SET_ZONE  2008
#define STATESERVER_OBJECT_CHANGE_ZONE  2009
#define STATESERVER_OBJECT_NOTFOUND  2015
#define STATESERVER_OBJECT_QUERY_ALL  2020
#define STATESERVER_QUERY_ZONE_OBJECT_ALL  2021
#define STATESERVER_OBJECT_LOCATE  2022
#define STATESERVER_OBJECT_LOCATE_RESP  2023
#define STATESERVER_OBJECT_QUERY_FIELD  2024
#define STATESERVER_OBJECT_QUERY_ALL_RESP  2030
#define STATESERVER_OBJECT_LEAVING_AI_INTEREST  2033
#define STATESERVER_OBJECT_SET_AI_CHANNEL  2045
#define STATESERVER_QUERY_ZONE_OBJECT_ALL_DONE  2046
#define STATESERVER_OBJECT_NOTIFY_MANAGING_AI 2047
#define STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT  2050
#define STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT  2051
#define STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT_RESP  2052
#define STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT_RESP  2053
#define STATESERVER_OBJECT_DELETE_DISK  2060
#define STATESERVER_SHARD_RESET  2061
#define STATESERVER_OBJECT_QUERY_FIELD_RESP  2062
#define STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED  2065
#define STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER  2066
#define STATESERVER_OBJECT_ENTER_AI_RECV  2067
#define STATESERVER_OBJECT_ENTER_OWNER_RECV  2068
#define STATESERVER_OBJECT_CHANGE_OWNER_RECV  2069
#define STATESERVER_OBJECT_SET_OWNER_RECV  2070
#define STATESERVER_OBJECT_QUERY_FIELDS  2080
#define STATESERVER_OBJECT_QUERY_FIELDS_RESP  2081
#define STATESERVER_OBJECT_QUERY_FIELDS_STRING  2082
#define STATESERVER_OBJECT_QUERY_MANAGING_AI  2083
#define STATESERVER_BOUNCE_MESSAGE  2086
#define STATESERVER_QUERY_OBJECT_CHILDREN_LOCAL  2087
#define STATESERVER_QUERY_OBJECT_CHILDREN_LOCAL_DONE  2089
#define STATESERVER_QUERY_OBJECT_CHILDREN_RESP  2087

#define LOCATION2CHANNEL(p, z) ((unsigned long long)(p)<<32|(unsigned long long)(z))
#pragma endregion

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
	unsigned long long m_owner;
	unsigned int m_parent_id;
	unsigned int m_zone_id;
	unsigned int m_do_id;
	DCClass *m_dclass;
	std::map<DCField*, std::string> m_fields;
	unsigned long long m_ai_channel;
	bool m_ai_explicitly_set;
	bool m_has_ram_fields;

	DistributedObject(unsigned int do_id, DCClass *dclass, unsigned int parent_id, unsigned int zone_id, DatagramIterator &dgi, bool has_other) :
	m_do_id(do_id), m_dclass(dclass), m_zone_id(zone_id),
	m_ai_channel(0), m_ai_explicitly_set(false), m_has_ram_fields(false)
	{
		for(int i = 0; i < m_dclass->get_num_inherited_fields(); ++i)
		{
			DCField *field = m_dclass->get_inherited_field(i);
			if(field->is_required() && !field->as_molecular_field())
			{
				UnpackFieldFromDG(field, dgi, m_fields[field]);
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
					m_has_ram_fields = true;
					UnpackFieldFromDG(field, dgi, m_fields[field]);
				}
				else
				{
					gLogger->error() << "Received non-RAM field "
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
				dg.add_data(m_fields[field]);
			}
		}
	}

	void append_other_data(Datagram &dg)
	{
		unsigned int field_count = 0;
		Datagram raw_fields;
		for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
		{
			if(it->first->is_ram() && !it->first->is_required())
			{
				field_count++;
				raw_fields.add_uint16(it->first->get_number());
				raw_fields.add_data(it->second);
			}
		}
		dg.add_uint16(field_count);
		dg.add_datagram(raw_fields);
	}

	void send_zone_entry()
	{
		Datagram dg(LOCATION2CHANNEL(m_parent_id, m_zone_id), m_do_id,
		            m_has_ram_fields ?
		            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER :
		            STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED);
		append_required_data(dg);
		if(m_has_ram_fields)
			append_other_data(dg);
		MessageDirector::singleton.handle_datagram(&dg, this);
	}

	void handle_parent_change(unsigned long long int new_parent)
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
			Datagram resp2(m_parent_id, m_do_id, STATESERVER_OBJECT_QUERY_MANAGING_AI);
			MessageDirector::singleton.handle_datagram(&resp2, this);
		}
	}

	void annihilate()
	{
		unsigned long long loc = LOCATION2CHANNEL(m_parent_id, m_zone_id);
		Datagram resp(loc, m_do_id, STATESERVER_OBJECT_DELETE_RAM);
		resp.add_uint32(m_do_id);
		MessageDirector::singleton.handle_datagram(&resp, this);
		distObjs[m_do_id] = NULL;
		gLogger->debug() << "DELETING THIS " << m_do_id << std::endl;
		delete this;
	}

	void handle_shard_reset()
	{
		// Tell my children:
		Datagram dg(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_SHARD_RESET);
		dg.add_uint64(m_ai_channel);
		MessageDirector::singleton.handle_datagram(&dg, this);
		// Fall over dead:
		annihilate();
	}

	virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
	{
		unsigned long long sender = dgi.read_uint64();
		unsigned short msgtype = dgi.read_uint16();
		switch(msgtype)
		{
			case STATESERVER_SHARD_RESET:
			{
				if(m_ai_channel != dgi.read_uint64()) {
					gLogger->warning() << m_do_id << " received reset for wrong"
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
				unsigned int field_id = dgi.read_uint16();
				std::string data;
				DCField *field = m_dclass->get_field_by_index(field_id);
				if(!field)
				{
					gLogger->error() << "Received update for missing field "
					                 << field_id << std::endl;
					break;
				}

				UnpackFieldFromDG(field, dgi, data);
				if(field->is_ram())
				{
					m_has_ram_fields = true;
				}
				if(field->is_required() || field->is_ram())
				{
					m_fields[field] = data;
				}

				std::set <unsigned long long int> targets;
				if(field->is_broadcast())
					targets.insert(LOCATION2CHANNEL(m_parent_id, m_zone_id));
				if(field->is_airecv())
					targets.insert(m_ai_channel);
				if(targets.size()) // TODO: Review this for efficiency?
				{
					Datagram resp(targets, sender, STATESERVER_OBJECT_UPDATE_FIELD);
					resp.add_uint32(m_do_id);
					resp.add_uint16(field_id);
					resp.add_data(data);
					MessageDirector::singleton.handle_datagram(&resp, this);
				}
				break;
			}
			case STATESERVER_OBJECT_NOTIFY_MANAGING_AI:
				gLogger->debug() << "STATESERVER_OBJECT_NOTIFY_MANAGING_AI " << m_parent_id << std::endl;
				if(m_ai_explicitly_set)
					break;
				// Fall through...
			case STATESERVER_OBJECT_SET_AI_CHANNEL:
			{
				gLogger->debug() << "STATESERVER_OBJECT_SET_AI_CHANNEL " << m_do_id << std::endl;
				unsigned int r_do_id = dgi.read_uint32();
				gLogger->debug() << "r_do_id " << r_do_id << std::endl;
				unsigned long long r_ai_channel = dgi.read_uint64();
				if(m_ai_channel == r_ai_channel)
					break;
				if((msgtype == STATESERVER_OBJECT_NOTIFY_MANAGING_AI && r_do_id == m_parent_id) || m_do_id == r_do_id)
				{
					if(m_ai_channel)
					{
						Datagram resp(m_ai_channel, m_do_id, STATESERVER_OBJECT_LEAVING_AI_INTEREST);
						resp.add_uint32(m_do_id);
						gLogger->debug() << m_do_id << " LEAVING AI INTEREST" << std::endl;
						MessageDirector::singleton.handle_datagram(&resp, this);
					}

					m_ai_channel = r_ai_channel;
					m_ai_explicitly_set = msgtype == STATESERVER_OBJECT_SET_AI_CHANNEL;

					Datagram resp(m_ai_channel, m_do_id, STATESERVER_OBJECT_ENTER_AI_RECV);
					append_required_data(resp);
					append_other_data(resp);
					MessageDirector::singleton.handle_datagram(&resp, this);
					gLogger->debug() << "Sending STATESERVER_OBJECT_ENTER_AI_RECV to " << m_ai_channel
						<< " from " << m_do_id << std::endl;

					Datagram resp2(LOCATION2CHANNEL(4030, m_do_id), m_do_id, STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
					resp2.add_uint32(m_do_id);
					resp2.add_uint64(m_ai_channel);
					MessageDirector::singleton.handle_datagram(&resp2, this);
				}
				break;
			}
			case STATESERVER_OBJECT_QUERY_MANAGING_AI:
			{
				gLogger->debug() << "STATESERVER_OBJECT_QUERY_MANAGING_AI" << std::endl;
				Datagram resp(sender, m_do_id, STATESERVER_OBJECT_NOTIFY_MANAGING_AI);
				resp.add_uint32(m_do_id);
				resp.add_uint64(m_ai_channel);
				MessageDirector::singleton.handle_datagram(&resp, this);
				break;
			}
			case STATESERVER_OBJECT_SET_ZONE:
			{
				unsigned int old_parent_id = m_parent_id, old_zone_id = m_zone_id;
				unsigned int new_parent_id = dgi.read_uint32();
				m_zone_id = dgi.read_uint32();

				std::set <unsigned long long int> targets;
				targets.insert(LOCATION2CHANNEL(old_parent_id, old_zone_id));
				if(m_ai_channel)
					targets.insert(m_ai_channel);
				Datagram resp3(targets, sender, STATESERVER_OBJECT_CHANGE_ZONE);
				resp3.add_uint32(m_do_id);
				resp3.add_uint32(new_parent_id);
				resp3.add_uint32(m_zone_id);
				resp3.add_uint32(old_parent_id);
				resp3.add_uint32(old_zone_id);
				MessageDirector::singleton.handle_datagram(&resp3, this);

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
				MessageDirector::singleton.handle_datagram(&dg, this);
				break;
			}
			case STATESERVER_OBJECT_QUERY_ALL:
			{
				Datagram dg(sender, m_do_id, STATESERVER_OBJECT_QUERY_ALL_RESP);
				dg.add_uint32(dgi.read_uint32()); // Copy context to response.
				append_required_data(dg);
				append_other_data(dg);
				MessageDirector::singleton.handle_datagram(&dg, this);
				break;
			}
			default:
				gLogger->warning() << "DistributedObject recv'd unkonw msgtype " << msgtype << std::endl;
		}
		return true;
	}
};

ConfigVariable<unsigned long long> cfg_channel("control", 0);

class StateServer : public Role
{
public:
	StateServer(RoleConfig roleconfig) : Role(roleconfig)
	{
		MessageDirector::singleton.subscribe_channel(this, cfg_channel.get_rval(m_roleconfig));
	}

	virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
	{
		unsigned long long sender = dgi.read_uint64();
		unsigned short msgtype = dgi.read_uint16();
		switch(msgtype)
		{
			case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED:
			{
				unsigned int parent_id = dgi.read_uint32();
				unsigned int zone_id = dgi.read_uint32();
				unsigned short dc_id = dgi.read_uint16();
				unsigned int do_id = dgi.read_uint32();

				distObjs[do_id] = new DistributedObject(do_id, gDCF->get_class(dc_id), parent_id, zone_id, dgi, false);
				break;
			}
			case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER:
			{
				unsigned int parent_id = dgi.read_uint32();
				unsigned int zone_id = dgi.read_uint32();
				unsigned short dc_id = dgi.read_uint16();
				unsigned int do_id = dgi.read_uint32();

				distObjs[do_id] = new DistributedObject(do_id, gDCF->get_class(dc_id), parent_id, zone_id, dgi, true);
				break;
			}
			case STATESERVER_SHARD_RESET:
			{
				unsigned long long int ai_channel = dgi.read_uint64();
				std::set <unsigned long long int> targets;
				for(auto it = distObjs.begin(); it != distObjs.end(); ++it)
					if(it->second && it->second->m_ai_channel == ai_channel && it->second->m_ai_explicitly_set)
						targets.insert(it->second->m_do_id);

				if(targets.size())
				{
					Datagram dg(targets, sender, STATESERVER_SHARD_RESET);
					dg.add_uint64(ai_channel);
					MessageDirector::singleton.handle_datagram(&dg, this);
				}
				break;
			}
			default:
				gLogger->error() << "StateServer recv'd unknown msgtype: " << msgtype << std::endl;
		}
		return true;
	}
};

RoleFactoryItem<StateServer> ss_fact("stateserver");
