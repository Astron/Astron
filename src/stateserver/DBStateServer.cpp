#include "core/global.h"
#include <unordered_set>

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
	set_con_name(name.str());
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

			// If object exists broadcast the delete message
			auto obj_keyval = m_objs.find(do_id);
			if(obj_keyval != m_objs.end())
			{
				DistributedObject* obj = obj_keyval->second;
				std::set<channel_t> targets;

				// Add location to broadcast
				if(obj->get_location())
				{
					targets.insert(obj->get_location());
				}

				// Add AI to broadcast
				if(obj->get_ai())
				{
					targets.insert(obj->get_ai());
				}

				// Add owner to broadcast
				if(obj->get_owner())
				{
					targets.insert(obj->get_owner());
				}

				// Build and send datagram
				Datagram dg(targets, sender, DBSS_OBJECT_DELETE_DISK);
				dg.add_uint32(do_id);
				send(dg);
			}

			// Send delete to database
			Datagram dg(m_db_channel, do_id, DBSERVER_OBJECT_DELETE);
			dg.add_uint32(do_id);
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_SET_FIELD:
		{
			uint32_t do_id = dgi.read_uint32();
			uint32_t field_id = dgi.read_uint16();

			DCField* field = g_dcf->get_field_by_index(field_id);
			if(field && field->is_db())
			{
				m_log->spam() << "Forwarding SetField for field with id " << field_id
				              << ", on object " << do_id << " to database." << std::endl;

				Datagram dg(m_db_channel, do_id, DBSERVER_OBJECT_SET_FIELD);
				dg.add_uint32(do_id);
				dg.add_uint16(field_id);
				dg.add_data(dgi.read_remainder());
				send(dg);
			}
			break;
		}
		case STATESERVER_OBJECT_SET_FIELDS:
		{
			uint32_t do_id = dgi.read_uint32();
			uint16_t field_count = dgi.read_uint16();

			std::unordered_map<DCField*, std::vector<uint8_t>> db_fields;
			for(uint16_t i=0; i<field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				DCField* field = g_dcf->get_field_by_index(field_id);
				if(!field)
				{
					m_log->warning() << "Received invalid field in SetFields"
					                 " with id " << field_id << std::endl;
					break;
				}
				if(field->is_db())
				{
					dgi.unpack_field(field, db_fields[field]);
				}
			}

			if(db_fields.size() > 0)
			{
				m_log->spam() << "Forwarding SetFields on object " << do_id << " to database." << std::endl;

				Datagram dg(m_db_channel, do_id, DBSERVER_OBJECT_SET_FIELDS);
				dg.add_uint32(do_id);
				dg.add_uint16(field_count);
				for(auto it = db_fields.begin(); it != db_fields.end(); ++it)
				{
					dg.add_uint16(it->first->get_number());
					dg.add_data(it->second);
				}
				send(dg);
			}

			break;
		}
		case STATESERVER_OBJECT_GET_FIELD:
		{
			uint32_t r_context = dgi.read_uint32();
			uint32_t r_do_id = dgi.read_uint32();
			uint16_t field_id = dgi.read_uint16();

			// If object is active or loading, the Object or Loader will handle it
			if(m_objs.find(r_do_id) != m_objs.end() ||
			   m_loading.find(r_do_id) != m_loading.end())
			{
				break;
			}

			m_log->spam() << "Received GetField for field with id " << field_id
			              << " on inactive object with id " << r_do_id << std::endl;

			// Check field is "ram db" or "required"
			DCField* field = g_dcf->get_field_by_index(field_id);
			if(!field || !(field->is_required() ||
			   (field->is_ram() && field->is_db())))
			{
				Datagram dg(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
				dg.add_uint32(r_context);
				dg.add_uint8(false);
				send(dg);
			}

			if(field->is_db())
			{
				// Get context for db query
				uint32_t db_context = m_next_context++;

				// Prepare reponse datagram
				/*bool ok = */m_context_datagrams.emplace(db_context, Datagram(sender, r_do_id,
					                                      STATESERVER_OBJECT_GET_FIELD_RESP));//->first;

				m_context_datagrams[db_context].add_uint32(r_context);

				// Send query to database
				Datagram dg(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_FIELD);
				dg.add_uint32(db_context);
				dg.add_uint32(r_do_id);
				dg.add_uint16(field_id);
				send(dg);
			}
			else // Field is required and not-db
			{
				Datagram dg = Datagram(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
				dg.add_uint32(r_context);
				dg.add_uint8(true);
				dg.add_uint16(field_id);
				dg.add_data(field->get_default_value());
				send(dg);
			}

			break;
		}
		case DBSERVER_OBJECT_GET_FIELD_RESP:
		{
			uint32_t db_context = dgi.read_uint32();

			// Check context
			auto dg_keyval = m_context_datagrams.find(db_context);
			if(dg_keyval == m_context_datagrams.end())
			{
				break; // Not meant for me, handled by LoadingObject
			}

			m_log->spam() << "Received GetFieldResp from database." << std::endl;

			// Get the datagram from the db_context
			Datagram dg(dg_keyval->second);

			// Cleanup the context
			m_context_datagrams.erase(db_context);

			// Check to make sure the datagram is appropriate
			DatagramIterator check_dgi = DatagramIterator(dg);
			uint16_t resp_type = check_dgi.get_msg_type();
			if(resp_type != STATESERVER_OBJECT_GET_FIELD_RESP)
			{
				if(resp_type == STATESERVER_OBJECT_GET_FIELDS_RESP)
				{
					m_log->warning() << "Received GetFieldsResp, but expecting GetFieldResp." << std::endl;
				}
				else if(resp_type == STATESERVER_OBJECT_GET_ALL_RESP)
				{
					m_log->warning() << "Received GetAllResp, but expecting GetFieldResp." << std::endl;
				}
				break;
			}

			// Add database field payload to response (don't know dclass, so must copy payload) and send
			dg.add_data(dgi.read_remainder());
			send(dg);

			break;
		}
		case STATESERVER_OBJECT_GET_FIELDS:
		{
			uint32_t r_context = dgi.read_uint32();
			uint32_t r_do_id = dgi.read_uint32();
			uint16_t field_count = dgi.read_uint16();

			// If object is active or loading, the Object or Loader will handle it
			if(m_objs.find(r_do_id) != m_objs.end() ||
			   m_loading.find(r_do_id) != m_loading.end())
			{
				break;
			}

			m_log->spam() << "Received GetFields for inactive object with id " << r_do_id << std::endl;

			// Read requested fields from datagram
			std::list<DCField*> db_fields; // Ram|required db fields in request
			std::list<DCField*> ram_fields; // Ram|required but not-db fields in request
			for(uint16_t i=0; i<field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				DCField* field = g_dcf->get_field_by_index(field_id);
				if(!field)
				{
					Datagram dg(sender, r_do_id, STATESERVER_OBJECT_GET_FIELDS_RESP);
					dg.add_uint32(r_context);
					dg.add_uint8(false);
					send(dg);
				}
				else if(field->is_ram() || field->is_required())
				{
					if(field->is_db())
					{
						db_fields.push_back(field);
					}
					else
					{
						ram_fields.push_back(field);
					}
				}
			}

			if(db_fields.size())
			{
				// Get context for db query
				uint32_t db_context = m_next_context++;

				// Prepare reponse datagram
				/*bool ok = */m_context_datagrams.emplace(db_context, Datagram(sender, r_do_id,
					                                      STATESERVER_OBJECT_GET_FIELDS_RESP));//->first;
				m_context_datagrams[db_context].add_uint32(r_context);
				m_context_datagrams[db_context].add_uint8(true);
				m_context_datagrams[db_context].add_uint16(ram_fields.size() + db_fields.size());
				for(auto it = ram_fields.begin(); it != ram_fields.end(); ++it)
				{
					m_context_datagrams[db_context].add_uint16((*it)->get_number());
					m_context_datagrams[db_context].add_data((*it)->get_default_value());
				}

				// Send query to database
				Datagram dg(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_FIELDS);
				dg.add_uint32(db_context);
				dg.add_uint32(r_do_id);
				dg.add_uint16(db_fields.size());
				for(auto it = db_fields.begin(); it != db_fields.end(); ++it)
				{
					dg.add_uint16((*it)->get_number());					
				}
				send(dg);
			}
			else // If no database fields exist
			{
				Datagram dg = Datagram(sender, r_do_id, STATESERVER_OBJECT_GET_FIELDS_RESP);
				dg.add_uint32(r_context);
				dg.add_uint8(true);
				dg.add_uint16(ram_fields.size());
				for(auto it = ram_fields.begin(); it != ram_fields.end(); ++it)
				{
					dg.add_uint16((*it)->get_number());
					dg.add_data((*it)->get_default_value());
				}
				send(dg);
			}

			break;
		}
		case DBSERVER_OBJECT_GET_FIELDS_RESP:
		{
			uint32_t db_context = dgi.read_uint32();

			// Check context
			auto dg_keyval = m_context_datagrams.find(db_context);
			if(dg_keyval == m_context_datagrams.end())
			{
				break; // Not meant for me, handled by LoadingObject
			}

			// Get the datagram from the db_context
			Datagram dg(dg_keyval->second);

			// Cleanup the context
			m_context_datagrams.erase(db_context);

			// Check to make sure the datagram is appropriate
			DatagramIterator check_dgi = DatagramIterator(dg);
			uint16_t resp_type = check_dgi.get_msg_type();
			if(resp_type != STATESERVER_OBJECT_GET_FIELDS_RESP)
			{
				if(resp_type == STATESERVER_OBJECT_GET_FIELD_RESP)
				{
					m_log->warning() << "Received GetFieldResp, but expecting GetFieldsResp." << std::endl;
				}
				else if(resp_type == STATESERVER_OBJECT_GET_ALL_RESP)
				{
					m_log->warning() << "Received GetAllResp, but expecting GetFieldsResp." << std::endl;
				}
				break;
			}

			m_log->spam() << "Received GetFieldResp from database." << std::endl;

			// Add database field payload to response (don't know dclass, so must copy payload).
			if(dgi.read_uint8() == true)
			{
				dgi.read_uint16(); // Discard field count
				dg.add_data(dgi.read_remainder());
			}
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

			m_log->spam() << "Received GetAll for inactive object with id " << r_do_id << std::endl;

			// Get context for db query, and remember caller with it
			uint32_t db_context = m_next_context++;

			/*bool ok = */m_context_datagrams.emplace(db_context, Datagram(sender, r_do_id,
					                                  STATESERVER_OBJECT_GET_ALL_RESP));//->first;

			m_context_datagrams[db_context].add_uint32(r_context);
			m_context_datagrams[db_context].add_uint32(r_do_id);
			m_context_datagrams[db_context].add_uint64(INVALID_CHANNEL); // Location

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
			auto dg_keyval = m_context_datagrams.find(db_context);
			if(dg_keyval == m_context_datagrams.end())
			{
				break; // Not meant for me, handled by LoadingObject
			}

			// Get the datagram from the db_context
			Datagram dg(dg_keyval->second);

			// Cleanup the context
			m_context_datagrams.erase(db_context);

			// Check to make sure the datagram is appropriate
			DatagramIterator check_dgi = DatagramIterator(dg);
			uint16_t resp_type = check_dgi.get_msg_type();
			if(resp_type != STATESERVER_OBJECT_GET_ALL_RESP)
			{
				if(resp_type == STATESERVER_OBJECT_GET_FIELD_RESP)
				{
					m_log->warning() << "Received GetFieldResp, but expecting GetAllResp." << std::endl;
				}
				else if(resp_type == STATESERVER_OBJECT_GET_FIELDS_RESP)
				{
					m_log->warning() << "Received GetFieldsResp, but expecting GetAllResp." << std::endl;
				}
				break;
			}

			m_log->spam() << "Received GetAllResp from database." << std::endl;

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

			// Add class to response
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

			break;
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
