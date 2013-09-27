#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"

static ConfigVariable<channel_t> control_channel("control", INVALID_CHANNEL);
static ConfigVariable<uint32_t> min_id("generate/min", INVALID_DO_ID);
static ConfigVariable<uint32_t> max_id("generate/max", UINT_MAX);
static ConfigVariable<std::string> engine_type("engine/type", "filesystem");

class DatabaseServer : public Role
{
	private:
		IDatabaseEngine *m_db_engine;
		LogCategory *m_log;

		channel_t m_control_channel;
		uint32_t m_min_id, m_max_id;
	public:
		DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
			m_db_engine(DBEngineFactory::singleton.instantiate(
			                engine_type.get_rval(roleconfig),
			                roleconfig["engine"],
			                min_id.get_rval(roleconfig),
			                max_id.get_rval(roleconfig))),
			m_control_channel(control_channel.get_rval(roleconfig)),
			m_min_id(min_id.get_rval(roleconfig)),
			m_max_id(max_id.get_rval(roleconfig))
		{
			// Initialize DatabaseServer log
			std::stringstream log_title;
			log_title << "Database(" << m_control_channel << ")";
			m_log = new LogCategory("db", log_title.str());

			// Check to see the engine was instantiated
			if(!m_db_engine)
			{
				m_log->fatal() << "No database engine of type '"
				               << engine_type.get_rval(roleconfig) << "' exists." << std::endl;
				exit(1);
			}

			// Listen on control channel
			subscribe_channel(m_control_channel);
		}

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{
			channel_t sender = dgi.read_uint64();
			uint16_t msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case DBSERVER_OBJECT_CREATE:
				{
					uint32_t context = dgi.read_uint32();

					// Start response with generic header
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_CREATE_RESP);
					resp.add_uint32(context);

					// Get DistributedClass
					uint16_t dc_id = dgi.read_uint16();
					DCClass *dcc = g_dcf->get_class(dc_id);
					if(!dcc)
					{
						m_log->error() << "Invalid DCClass when creating object: #" << dc_id << std::endl;
						resp.add_uint32(INVALID_DO_ID);
						send(resp);
						return;
					}

					// Unpack fields to be passed to database
					DatabaseObject dbo(dc_id);
					uint16_t field_count = dgi.read_uint16();
					m_log->spam() << "Unpacking fields..." << std::endl;
					try
					{
						for(uint32_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							DCField *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
								{
									dgi.unpack_field(field, dbo.fields[field]);
								}
								else
								{
									m_log->warning() << "Recieved non-db field in CREATE_STORED_OBJECT." << std::endl;
									dgi.skip_field(field);
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
						               << e.what() << std::endl;

						resp.add_uint32(INVALID_DO_ID);
						send(resp);
						return;
					}

					// Check for required fields, and populate with defaults
					m_log->spam() << "Checking all required fields exist..." << std::endl;
					for(int i = 0; i < dcc->get_num_inherited_fields(); ++i)
					{
						DCField *field = dcc->get_inherited_field(i);
						if(field->is_required() && field->is_db() && !field->as_molecular_field()
						        && dbo.fields.find(field) == dbo.fields.end())
						{
							if(!field->has_default_value())
							{
								m_log->error() << "Field " << field->get_name()
								               << " missing when trying to create object"
								               << " of type " << dcc->get_name() << std::endl;
								resp.add_uint32(INVALID_DO_ID);
								send(resp);
								return;
							}
							else
							{
								std::string val = field->get_default_value();
								dbo.fields[field] = vector<uint8_t>(val.begin(), val.end());
								if(!field->has_default_value())
								{
									m_log->error() << "Field " << field->get_name() << " missing when trying to create "
									               "object of type " << dcc->get_name();

									resp.add_uint32(0);
									send(resp);
									return;
								}
								else
								{
									std::string val = field->get_default_value();
									dbo.fields[field] = vector<uint8_t>(val.begin(), val.end());
								}
							}
						}
					}

					// Create object in database
					m_log->spam() << "Creating stored object..." << std::endl;
					uint32_t do_id = m_db_engine->create_object(dbo);
					if(do_id == INVALID_DO_ID || do_id < m_min_id || do_id > m_max_id)
					{
						m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_uint32(INVALID_DO_ID);
						send(resp);
						return;
					}

					m_log->spam() << "... created with ID: " << do_id << std::endl;
					resp.add_uint32(do_id);
					send(resp);
				}
				break;
				case DBSERVER_OBJECT_GET_ALL:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_ALL_RESP);
					resp.add_uint32(context);

					uint32_t do_id = dgi.read_uint32();

					m_log->spam() << "Selecting all from do_id: " << do_id << "... " << std::endl;

					DatabaseObject dbo;
					if(m_db_engine->get_object(do_id, dbo))
					{
						m_log->spam() << "... object found!" << std::endl;
						resp.add_uint8(SUCCESS);
						resp.add_uint16(dbo.dc_id);
						resp.add_uint16(dbo.fields.size());
						for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
						{
							resp.add_uint16(it->first->get_number());
							resp.add_data(it->second);
							m_log->spam() << "Recieved field id-" << it->first->get_number() << ", value-" << std::string(
							                  it->second.begin(), it->second.end()) << std::endl;
						}
					}
					else
					{
						m_log->spam() << "... object not found." << std::endl;
						resp.add_uint8(FAILURE);
					}
					send(resp);
				}
				break;
				case DBSERVER_OBJECT_DELETE:
				{
					uint32_t do_id = dgi.read_uint32();
					m_db_engine->delete_object(do_id);
					m_log->debug() << "Deleted object with ID: " << do_id << std::endl;
				}
				break;
				case DBSERVER_OBJECT_SET_FIELD:
				{
					uint32_t do_id = dgi.read_uint32();
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						return;
					}

					uint16_t field_id = dgi.read_uint16();
					DCField *field = dcc->get_field_by_index(field_id);
					if(field && field->is_db())
					{
						std::vector<uint8_t> value;
						dgi.unpack_field(field, value);
						m_db_engine->set_field(do_id, field, value);
					}
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS:
				{
					uint32_t do_id = dgi.read_uint32();
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						return;
					}

					std::map<DCField*, std::vector<uint8_t>> fields;
					uint16_t field_count = dgi.read_uint16();
					m_log->spam() << "Unpacking fields..." << std::endl;
					try
					{
						for(uint32_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							DCField *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
								{
									dgi.unpack_field(field, fields[field]);
								}
								else
								{
									dgi.skip_field(field);
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
						               << e.what() << std::endl;
						return;
					}
					m_db_engine->set_fields(do_id, fields);
				}
				break;
				case DBSERVER_OBJECT_SET_FIELD_IF_EQUALS:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP);
					resp.add_uint32(context);

					uint32_t do_id = dgi.read_uint32();
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
					}

					uint16_t field_id = dgi.read_uint16();
					DCField *field = dcc->get_field_by_index(field_id);
					if(!field->is_db())
					{
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					std::vector<uint8_t> equal;
					std::vector<uint8_t> value;
					dgi.unpack_field(field, equal);
					dgi.unpack_field(field, value);

					if(m_db_engine->set_field_if_equals(do_id, field, equal, value))
					{
						resp.add_uint8(SUCCESS);
						send(resp);
						return;
					}

					resp.add_uint8(FAILURE);
					if(value.size() > 0)
					{
						resp.add_uint16(field_id);
						resp.add_data(value);
					}
					send(resp);
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP);
					resp.add_uint32(context);

					uint32_t do_id = dgi.read_uint32();
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Unpack fields from datagram
					std::map<DCField*, std::vector<uint8_t>> equals;
					std::map<DCField*, std::vector<uint8_t>> values;
					uint16_t field_count = dgi.read_uint16();
					try
					{
						for(uint16_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							DCField *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
								{
									dgi.unpack_field(field, equals[field]);
									dgi.unpack_field(field, values[field]);
								}
								else
								{
									resp.add_uint8(FAILURE);
									send(resp);
									return;
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated."
						               << " e.what(): " << e.what() << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					if(m_db_engine->set_fields_if_equals(do_id, equals, values))
					{
						resp.add_uint8(SUCCESS);
						send(resp);
						return;
					}

					resp.add_uint8(FAILURE);
					if(values.size() > 0)
					{
						resp.add_uint16(values.size());
						for(auto it = values.begin(); it != values.end(); ++it)
						{
							resp.add_uint16(it->first->get_number());
							resp.add_data(it->second);
						}
					}
					send(resp);
				}
				break;
				case DBSERVER_OBJECT_GET_FIELD:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELD_RESP);
					resp.add_uint32(context);

					// Get object id from datagram
					uint32_t do_id = dgi.read_uint32();
					m_log->spam() << "Reading field from obj-" << do_id << "..." << std::endl;

					// Get object class from db
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						m_log->spam() << "... object not found in database." << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Get field from datagram
					uint16_t field_id = dgi.read_uint16();
					DCField *field = dcc->get_field_by_index(field_id);
					if(!field)
					{
						m_log->error() << "Asked for invalid field,"
						               << " reading from obj-" << do_id << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Get value from database
					std::vector<uint8_t> value;
					if(!m_db_engine->get_field(do_id, field, value))
					{
						m_log->spam() << "... field not set." << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Send value in response
					resp.add_uint8(SUCCESS);
					resp.add_uint16(field_id);
					resp.add_data(value);
					send(resp);
					m_log->spam() << "... success." << std::endl;
				}
				break;
				case DBSERVER_OBJECT_GET_FIELDS:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELDS_RESP);
					resp.add_uint32(context);

					// Get object id from datagram
					uint32_t do_id = dgi.read_uint32();
					m_log->spam() << "Reading fields from obj-" << do_id << "..." << std::endl;

					// Get object class from db
					DCClass *dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Get fields from datagram
					uint16_t field_count = dgi.read_uint16();
					std::vector<DCField*> fields(field_count);
					for(uint16_t i = 0; i < field_count; ++i)
					{
						uint16_t field_id = dgi.read_uint16();
						DCField *field = dcc->get_field_by_index(field_id);
						if(!field)
						{
							m_log->error() << "Asked for invalid field(s),"
							               << " reading from object #" << do_id << std::endl;
							resp.add_uint8(FAILURE);
							send(resp);
							return;
						}
						fields[i] = field;
					}

					// Get values from database
					std::map<DCField*, std::vector<uint8_t>> values;
					if(!m_db_engine->get_fields(do_id, fields, values))
					{
						m_log->spam() << "... failure." << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Send value in response
					resp.add_uint8(SUCCESS);
					resp.add_uint16(values.size());
					for(auto it = values.begin(); it != values.end(); ++it)
					{
						resp.add_uint16(it->first->get_number());
						resp.add_data(it->second);
					}
					send(resp);
					m_log->spam() << "... success." << std::endl;
				}
				break;
				case DBSERVER_OBJECT_DELETE_FIELD:
				{
					uint32_t do_id = dgi.read_uint32();
					m_log->spam() << "Deleting field of obj-" << do_id << "..." << std::endl;

					DCClass* dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						m_log->spam() << "... object does not exist." << std::endl;
						return;
					}

					uint16_t field_id = dgi.read_uint16();
					DCField* field = dcc->get_field_by_index(field_id);
					if(!field)
					{
						m_log->error() << "Tried to delete an invalid field:" << field_id
						               << " on obj-" << do_id << "." << std::endl;
						return;
					}

					if(field->is_db())
					{
						if(field->has_default_value())
						{
							std::string str = field->get_default_value();
							std::vector<uint8_t> value(str.begin(), str.end());
							/* Alternate implementation for performance compare */
							//std::vector<uint8_t> value(str.length());
							//memcpy(&value[0], str.c_str(), str.length());
							m_db_engine->set_field(do_id, field, value);
							m_log->spam() << "... field set to default." << std::endl;
						}
						else if(!field->is_required())
						{
							m_db_engine->del_field(do_id, field);
							m_log->spam() << "... field deleted." << std::endl;
						}
						else
						{
							m_log->warning() << "Cannot delete required field of obj-" << do_id
							                 << "." << std::endl;
						}
					}
					else
					{
						m_log->warning() << "Cannot delete non-db field of obj-" << do_id
						                 << "." << std::endl;
					}
				}
				break;
				case DBSERVER_OBJECT_DELETE_FIELDS:
				{
					uint32_t do_id = dgi.read_uint32();
					m_log->spam() << "Deleting field of obj-" << do_id << "..." << std::endl;

					DCClass* dcc = m_db_engine->get_class(do_id);
					if(!dcc)
					{
						m_log->spam() << "... object does not exist." << std::endl;
						return;
					}

					uint16_t field_count = dgi.read_uint16();
					std::vector<DCField*> del_fields;
					std::map<DCField*, std::vector<uint8_t>> set_fields;
					for(uint16_t i = 0; i < field_count; ++i)
					{
						uint16_t field_id = dgi.read_uint16();
						DCField* field = dcc->get_field_by_index(field_id);
						if(!field)
						{
							m_log->error() << "Tried to delete an invalid field:" << field_id
							               << " on obj-" << do_id << "." << std::endl;
							return;
						}
						else if(field->is_db())
						{
							if(field->has_default_value())
							{
								std::string str = field->get_default_value();
								std::vector<uint8_t> value(str.begin(), str.end());
								/* Alternate implementation for performance compare */
								//std::vector<uint8_t> value(str.length());
								//memcpy(&value[0], str.c_str(), str.length());
								set_fields[field] = value;
								m_log->spam() << "... field set to default ..." << std::endl;
							}
							else if(!field->is_required())
							{
								del_fields.push_back(field);
								m_log->spam() << "... field deleted ..." << std::endl;
							}
							else
							{
								m_log->warning() << "Cannot delete required field of obj-" << do_id
								                 << "." << std::endl;
							}
						}
						else
						{
							m_log->warning() << "Cannot delete non-db field of obj-" << do_id
							                 << "." << std::endl;
						}
					}

					if(del_fields.size() > 0)
					{
						m_db_engine->del_fields(do_id, del_fields);
						m_log->spam() << "... fields deleted." << std::endl;
					}
					if(set_fields.size() > 0)
					{
						m_db_engine->set_fields(do_id, set_fields);
						m_log->spam() << "... fields deleted." << std::endl;
					}
				}
				break;
				default:
					m_log->error() << "Recieved unknown MsgType: " << msg_type << std::endl;
			};
		}

};

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
