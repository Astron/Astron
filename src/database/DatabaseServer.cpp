#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"
#include <fstream>

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> id_min("generate/min", 0);
ConfigVariable<unsigned int> id_max("generate/max", UINT_MAX);
ConfigVariable<std::string> engine_type("engine/type", "filesystem");

class DatabaseServer : public Role
{
	private:
		IDatabaseEngine *m_db_engine;
		LogCategory *m_log;

		channel_t m_control_channel;
		unsigned int m_id_min, m_id_max;
	public:
		DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
			m_db_engine(DBEngineFactory::singleton.instantiate(
							engine_type.get_rval(roleconfig),
							roleconfig["engine"],
							id_min.get_rval(roleconfig))),
			m_control_channel(control_channel.get_rval(roleconfig)),
			m_id_min(id_min.get_rval(roleconfig)),
			m_id_max(id_max.get_rval(roleconfig))
		{
			std::stringstream log_title;
			log_title << "Database(" << m_control_channel << ")";
			m_log = new LogCategory("db", log_title.str());
			if(!m_db_engine)
			{
				m_log->fatal() << "No database engine of type '" << engine_type.get_rval(roleconfig) << "' exists." << std::endl;
				exit(1);
			}

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

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
					resp.add_uint32(context);

					unsigned short dc_id = dgi.read_uint16();
					DCClass *dcc = gDCF->get_class(dc_id);
					if(!dcc)
					{
						m_log->error() << "Invalid DCClass when creating object. #" << dc_id << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					unsigned short field_count = dgi.read_uint16();

					unsigned int do_id = m_db_engine->get_next_id();
					if(do_id > m_id_max || do_id == 0)
					{
						m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					DatabaseObject dbo;
					dbo.do_id = do_id;
					dbo.dc_id = dc_id;

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
									std::string tmp;
									dgi.unpack_field(field, tmp);
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
									dbo.fields[field] = field->get_default_value();
								}
							}
						}
					}

					m_log->spam() << "Creating stored object with ID: " << do_id << " ..." << std::endl;
					if(m_db_engine->create_object(dbo))
					{
						resp.add_uint32(do_id);
					}
					else
					{
						resp.add_uint32(0);
					}

					send(resp);
				}
				break;
				case DBSERVER_SELECT_STORED_OBJECT_ALL:
				{
					unsigned int context = dgi.read_uint32();

					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_SELECT_STORED_OBJECT_ALL_RESP);
					resp.add_uint32(context);

					DatabaseObject dbo;
					dbo.do_id = dgi.read_uint32();
					if(m_db_engine->get_object(dbo))
					{
						resp.add_uint8(1);
						resp.add_uint16(dbo.dc_id);
						resp.add_uint16(dbo.fields.size());
						for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
						{
							resp.add_uint16(it->first->get_number());
							resp.add_data(it->second);
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
