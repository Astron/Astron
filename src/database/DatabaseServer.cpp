#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"

ConfigVariable<channel_t> control_channel("control", INVALID_CHANNEL);
ConfigVariable<uint32_t> min_id("generate/min", INVALID_DO_ID);
ConfigVariable<uint32_t> max_id("generate/max", UINT_MAX);
ConfigVariable<std::string> engine_type("engine/type", "filesystem");

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
				m_log->fatal() << "No database engine of type '" << engine_type.get_rval(roleconfig) << "' exists." << std::endl;
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
						if(field->is_required() && field->is_db() && !field->as_molecular_field())
						{
							if(dbo.fields.find(field) == dbo.fields.end())
							{
								if(!field->has_default_value())
								{
									m_log->error() << "Field " << field->get_name() << " missing when trying to create "
										"object of type " << dcc->get_name() << std::endl;

									resp.add_uint32(INVALID_DO_ID);
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
							m_log->spam() << "Recieved field id-" << it->first->get_number() << ", value-" << std::string(it->second.begin(), it->second.end()) << std::endl; 
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
					DCClass *dcc = m_db_engine->get_dclass(do_id);
					if(!dcc)
					{
						return;
					}

					DatabaseObject dbo(dcc->get_number());
					uint16_t field_id = dgi.read_uint16();
					DCField *field = dcc->get_field_by_index(field_id);
					if(field)
					{
						if(field->is_db())
						{
							dgi.unpack_field(field, dbo.fields[field]);
							m_db_engine->set_fields(do_id, dbo);
						}
					}
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS:
				{
					uint32_t do_id = dgi.read_uint32();
					DCClass *dcc = m_db_engine->get_dclass(do_id);
					if(!dcc)
					{
						return;
					}

					DatabaseObject dbo(dcc->get_number());
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
									m_log->warning() << "Recieved non-db field in OBJECT_SET_FIELDS." << std::endl;
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
					m_db_engine->set_fields(do_id, dbo);					
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS:
				{
					uint32_t context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP);
					resp.add_uint32(context);

					// Get existing database values
					DatabaseObject dbo_get;
					uint32_t do_id = dgi.read_uint32();
					if(!m_db_engine->get_object(do_id, dbo_get))
					{
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Unpack fields from datagram
					DCClass *dcc = g_dcf->get_class(dbo_get.dc_id);
					DatabaseObject dbo_old(dbo_get.dc_id);
					DatabaseObject dbo_new(dbo_get.dc_id);
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
									dgi.unpack_field(field, dbo_old.fields[field]);
									dgi.unpack_field(field, dbo_new.fields[field]);
								}
								else
								{
									m_log->error() << "Recieved non-db field in OBJECT_SET_FIELDS_IF_EQUALS." << std::endl;
									resp.add_uint8(FAILURE);
									send(resp);
									return;
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
						               << e.what() << std::endl;
						resp.add_uint8(FAILURE);
						send(resp);
						return;
					}

					// Compare old and existing fields
					bool failure = false;
					std::list<DCField*> not_equals;
					for(auto it = dbo_old.fields.begin(); it != dbo_old.fields.end() && !failure; ++it)
					{
						auto current = dbo_get.fields.find(it->first);
						if(current != dbo_get.fields.end())
						{
							if(current->second != it->second)
							{
								failure = true;
								not_equals.push_back(current->first);
							}
						}
						else
						{
							failure = true;
						}
					}

					// Set fields if successful
					if(!failure)
					{
						m_db_engine->set_fields(do_id, dbo_new);
						resp.add_uint8(SUCCESS);
						send(resp);
						return;
					}

					// Return current fields if not equals
					resp.add_uint16(not_equals.size());
					for(auto it = not_equals.begin(); it != not_equals.end(); ++it)
					{
						std::vector<uint8_t> data = dbo_get.fields[*it];
						resp.add_uint16((*it)->get_number());
						resp.add_data(data);
					}
					send(resp);
				}
				break;
				default:
					m_log->error() << "Recieved unknown MsgType: " << msg_type << std::endl;
			};
		}
};

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
