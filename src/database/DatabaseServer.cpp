#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"
#include <fstream>

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> id_start("generate/min", 1000);
ConfigVariable<unsigned int> id_end("generate/max", 2000);
ConfigVariable<std::string> engine_type("engine/type", "filesystem");

class DatabaseServer : public Role
{
	private:
		IDatabaseEngine *m_db_engine;
		LogCategory *m_log;
	public:
		DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
			m_db_engine(DBEngineFactory::singleton.instantiate(engine_type.get_rval(roleconfig), roleconfig["engine"],
				id_start.get_rval(roleconfig)))
		{
			std::stringstream ss;
			ss << "Database CH: " << control_channel.get_rval(m_roleconfig);
			m_log = new LogCategory("db", ss.str());
			if(!m_db_engine)
			{
				m_log->fatal() << "No database engine of type '" << engine_type.get_rval(roleconfig) << "' exists." << std::endl;
				exit(1);
			}


			subscribe_channel(control_channel.get_rval(m_roleconfig));
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
					resp.add_server_header(sender, control_channel.get_rval(m_roleconfig), DBSERVER_CREATE_STORED_OBJECT_RESP);
					resp.add_uint32(context);

					DCClass *dcc = gDCF->get_class(dgi.read_uint16());
					if(!dcc)
					{
						m_log->error() << "Unknown dcclass" << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					unsigned short field_count = dgi.read_uint16();

					unsigned int do_id = m_db_engine->get_next_id();
					if(do_id > id_end.get_rval(m_roleconfig) || do_id == 0)
					{
						m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					DatabaseObject dbo;
					dbo.do_id = do_id;
					dbo.dc_id = dcc->get_number();

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
					for(unsigned int i = 0; i < dcc->get_num_inherited_fields(); ++i)
					{
						DCField *field = dcc->get_inherited_field(i);
						if(field->is_required() && field->is_db() && !field->as_molecular_field())
						{
							if(dbo.fields.find(field) == dbo.fields.end())
							{
								m_log->error() << "Field " << field->get_name() << " missing when trying to create "
									"object of type " << dcc->get_name();

								resp.add_uint32(0);
								send(resp);
								return;
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
					resp.add_server_header(sender, control_channel.get_rval(m_roleconfig), DBSERVER_SELECT_STORED_OBJECT_ALL_RESP);
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
					if(dgi.read_uint32() == 'Die!')
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
