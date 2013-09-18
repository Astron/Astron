#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> min_id("generate/min", 0);
ConfigVariable<unsigned int> max_id("generate/max", UINT_MAX);
ConfigVariable<std::string> engine_type("engine/type", "filesystem");

class DatabaseServer : public Role
{
	private:
		IDatabaseEngine *m_db_engine;
		LogCategory *m_log;

		channel_t m_control_channel;
		unsigned int m_min_id, m_max_id;
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
			unsigned short msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case DBSERVER_CREATE_STORED_OBJECT:
				{
					unsigned int context = dgi.read_uint32();

					// Start response with generic header
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
					resp.add_uint32(context);

					// Get DistributedClass
					unsigned short dc_id = dgi.read_uint16();
					DCClass *dcc = gDCF->get_class(dc_id);
					if(!dcc)
					{
						m_log->error() << "Invalid DCClass when creating object: #" << dc_id << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					// Unpack fields to be passed to database
					DatabaseObject dbo(dc_id);
					unsigned short field_count = dgi.read_uint16();
					m_log->spam() << "Unpacking fields..." << std::endl;
					try
					{
						for(unsigned int i = 0; i < field_count; ++i)
						{
							unsigned short field_id = dgi.read_uint16();
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

						resp.add_uint32(0);
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
										"object of type " << dcc->get_name();

									resp.add_uint32(0);
									send(resp);
									return;
								}
								else
								{
									std::string val = field->get_default_value();
									dbo.fields[field] = bytes(val.begin(), val.end());
								}
							}
						}
					}

					// Create object in database
					m_log->spam() << "Creating stored object..." << std::endl;
					unsigned int do_id = m_db_engine->create_object(dbo);
					if(do_id == 0 || do_id < m_min_id || do_id > m_max_id)
					{
						m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					m_log->spam() << "... created with ID: " << do_id << std::endl;
					resp.add_uint32(do_id);
					send(resp);
				}
				break;
				case DBSERVER_SELECT_STORED_OBJECT_ALL:
				{
					unsigned int context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP);
					resp.add_uint32(context);

					unsigned int do_id = dgi.read_uint32();

					DatabaseObject dbo;
					if(m_db_engine->get_object(do_id, dbo))
					{
						resp.add_uint8(1);
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
						resp.add_uint8(0);
					}
					send(resp);
				}
				break;
				case DBSERVER_DELETE_STORED_OBJECT:
				{
					if(dgi.read_uint32() == DBSERVER_DELETE_STORED_OBJECT_VERIFY_CODE)
					{
						unsigned int do_id = dgi.read_uint32();
						m_db_engine->delete_object(do_id);
						m_log->debug() << "Deleted object with ID: " << do_id << std::endl;
					}
					else
					{
						m_log->warning() << "Wrong delete verify code." << std::endl;
					}
				}
				break;
				default:
					m_log->error() << "Recieved unknown MsgType: " << msg_type << std::endl;
			};
		}
};

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
