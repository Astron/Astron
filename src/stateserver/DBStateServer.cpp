#include "core/global.h"

#include "DBStateServer.h"
#include "LoadingObject.h"

// RoleConfig
static ConfigVariable<channel_t> database_channel("database", INVALID_CHANNEL);

// RangesConfig
static ConfigVariable<uint32_t> range_min("min", INVALID_DO_ID);
static ConfigVariable<uint32_t> range_max("max", UINT32_MAX);

DBStateServer::DBStateServer(RoleConfig roleconfig) : StateServer(roleconfig),
	m_db_channel(database_channel.get_rval(m_roleconfig)), m_next_context(0)
{
	RangesConfig ranges = roleconfig["ranges"];
	for(auto it = ranges.begin(); it != ranges.end(); ++it)
	{
		channel_t min = range_min.get_rval(*it);
		channel_t max = range_max.get_rval(*it);
		MessageDirector::singleton.subscribe_range(this, min, max);
	}

	std::stringstream name;
	name << "DBSS(Database: " << m_db_channel << ")";
	m_log = new LogCategory("dbss", name.str());
}

DBStateServer::~DBStateServer()
{
	delete m_log;
}

void DBStateServer::handle_activate(DatagramIterator &dgi, bool has_other)
{
	uint32_t do_id = dgi.read_uint32();
	uint32_t parent_id = dgi.read_uint32();
	uint32_t zone_id = dgi.read_uint32();

	// Check object is not already active
	if(m_objs.find(do_id) != m_objs.end() || m_loading.find(do_id) != m_loading.end())
	{
		m_log->warning() << "Received activate for already-active object"
		                 << " - id:" << do_id << std::endl;
		return;
	}

	if(!has_other)
	{
		m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id);
	}
	else
	{
		uint16_t dc_id = dgi.read_uint16();

		// Check dclass is valid
		if(dc_id >= g_dcf->get_num_classes())
		{
			m_log->error() << "Received activate_other with unknown dclass"
		                 << " - id:" << dc_id << std::endl;
			return;
		}

		DCClass *dclass = g_dcf->get_class(dc_id);
		m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id, dclass, dgi);
	}
}

void DBStateServer::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS:
		{
			handle_activate(dgi, false);
			break;
		}
		case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER:
		{
			handle_activate(dgi, true);
			break;
		}
		case DBSS_OBJECT_DELETE_DISK:
		{
			uint32_t do_id = dgi.read_uint32();
			auto obj_keyval = m_objs.find(do_id);
			if(obj_keyval != m_objs.end())
			{
				// TODO: Handle broadcast behavior
			}

			Datagram dg(m_db_channel, do_id, DBSERVER_OBJECT_DELETE);
			dg.add_uint32(do_id);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_ALL:
		{
			uint32_t r_context = dgi.read_uint32();
			uint32_t r_do_id = dgi.read_uint32();

			// If object is active or loading, the Object or Loader will handle it
			if(m_objs.find(r_do_id) != m_objs.end() ||
			   m_loading.find(r_do_id) != m_loading.end())
			{
				break;
			}

			m_log->spam() << "Received GetAll for inactive object with id " << r_do_id
			              << ", sending query to database." << std::endl;

			// Get context for db query, and remember reply with it
			uint32_t db_context = m_next_context++;
			m_resp_context[db_context] = GetRecord(sender, r_context, r_do_id);

			// Send query to database
			Datagram dg(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_ALL);
			dg.add_uint32(db_context);
			dg.add_uint32(r_do_id);
			send(dg);

			break;
		}
		case DBSERVER_OBJECT_GET_ALL_RESP:
		{
			uint32_t db_context = dgi.read_uint32();

			// Check context
			auto caller_keyval = m_resp_context.find(db_context);
			if(caller_keyval == m_resp_context.end())
			{
				break; // Not meant for me, handled by LoadingObject
			}
			GetRecord caller = caller_keyval->second;

			m_log->spam() << "Received GetAllResp from database"
			              " for object with id " << caller.do_id << std::endl;

			// Cleanup the context first, so it is removed if any exceptions occur
			m_resp_context.erase(db_context);

			// If object not found, just cleanup the context map
			if(dgi.read_uint8() != true)
			{
				break; // Object not found
			}

			// Read object class
			uint16_t dc_id = dgi.read_uint16();
			if(!dc_id)
			{
				m_log->error() << "Received object from database with unknown dclass"
				               << " - id:" << dc_id << std::endl;
				break;
			}
			DCClass* r_dclass = g_dcf->get_class(dc_id);

			// Get fields from database
			std::unordered_map<DCField*, std::vector<uint8_t>> required_fields;
			std::unordered_map<DCField*, std::vector<uint8_t>> ram_fields;
			if(!unpack_db_fields(dgi, r_dclass, required_fields, ram_fields))
			{
				m_log->error() << "Error while unpacking fields from database." << std::endl;
				break;
			}

			// Prepare SSGetAllResp
			Datagram dg(caller.sender, caller.do_id, STATESERVER_OBJECT_GET_ALL_RESP);
			dg.add_uint32(caller.do_id);
			dg.add_uint32(INVALID_DO_ID);
			dg.add_uint32(INVALID_ZONE);
			dg.add_uint16(r_dclass->get_number());

			// Add required fields to datagram
			int dcc_field_count = r_dclass->get_num_inherited_fields();
			for(int i = 0; i < dcc_field_count; ++i)
			{
				DCField *field = r_dclass->get_inherited_field(i);
				if(!field->as_molecular_field() && field->is_required())
				{
					auto req_it = required_fields.find(field);
					if(req_it != required_fields.end())
					{
						dg.add_data(req_it->second);
					}
					else
					{
						dg.add_data(field->get_default_value());
					}
				}
			}

			// Add ram fields to datagram
			dg.add_uint16(ram_fields.size());
			for(auto it = ram_fields.begin(); it != ram_fields.end(); ++it)
			{
				dg.add_uint16(it->first->get_number());
				dg.add_data(it->second);
			}

			// Send response back to caller
			send(dg);
		}
		default:
		{
			if(msgtype < STATESERVER_MSGTYPE_MIN || msgtype > DBSERVER_MSGTYPE_MAX)
			{
				m_log->warning() << "Received unknown message of type " << msgtype << std::endl;
			}
			else
			{
				m_log->spam() << "Ignoring stateserver or database message"
				              << " of type " << msgtype << std::endl;
			}
		}
	}
}

void DBStateServer::receive_object(DistributedObject* obj)
{
	m_objs[obj->get_id()] = obj;
}
void DBStateServer::discard_loader(uint32_t do_id)
{
	m_loading.erase(do_id);
}

bool unpack_db_fields(DatagramIterator &dgi, DCClass* dclass,
                      std::unordered_map<DCField*, std::vector<uint8_t>> &required,
                      std::unordered_map<DCField*, std::vector<uint8_t>> &ram)
{
	// Unload ram and required fields from database resp
	uint16_t db_field_count = dgi.read_uint16();
	for(uint16_t i = 0; i < db_field_count; ++i)
	{
		uint16_t field_id = dgi.read_uint16();
		DCField *field = dclass->get_field_by_index(field_id);
		if(!field)
		{
			return false;
		}
		if(field->is_ram())
		{
			dgi.unpack_field(field, ram[field]);
		}
		else if(field->is_required())
		{
			dgi.unpack_field(field, required[field]);
		}
		else
		{
			dgi.skip_field(field);
		}
	}

	return true;
}

RoleFactoryItem<DBStateServer> dbss_fact("dbss");
