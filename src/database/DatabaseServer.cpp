#include "core/global.h"
#include "core/RoleFactory.h"
#include "DBEngineFactory.h"
#include "IDatabaseEngine.h"
#include <fstream>

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> id_start("generate/min", 1000);
ConfigVariable<unsigned int> id_end("generate/max", 2000);
ConfigVariable<std::string> engine_type("engine/type", "filesystem");
LogCategory db_log("db", "Database");

class DatabaseServer : public Role
{
	private:
		IDatabaseEngine *m_db_engine;
	public:
		DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
			m_db_engine(DBEngineFactory::singleton.instantiate(engine_type.get_rval(roleconfig), roleconfig["engine"],
				id_start.get_rval(roleconfig)))
		{
			if(!m_db_engine)
			{
				db_log.fatal() << "No database engine of type '" << engine_type.get_rval(roleconfig) << "' exists." << std::endl;
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
						db_log.error() << "Unknown dcclass" << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					unsigned short field_count = dgi.read_uint16();

					unsigned int do_id = m_db_engine->get_next_id();
					if(do_id > id_end.get_rval(m_roleconfig) || do_id == 0)
					{
						db_log.error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_uint32(0);
						send(resp);
						return;
					}

					db_log.spam() << "Unpacking fields..." << std::endl;
					std::map<DCField*, std::string> fields;
					try
					{
						unsigned short field_id = dgi.read_uint16();
						DCField *field = dcc->get_field_by_index(field_id);
						if(field)
						{
							dgi.unpack_field(field, fields[field]);
						}
					}
					catch(std::exception &e)
					{
						db_log.error() << "Error while unpacking fields, msg may be truncated. e.what(): "
							<< e.what() << std::endl;
					}

					db_log.spam() << "Creating stored object with ID: " << do_id << " ..." << std::endl;

					if(m_db_engine->create_object(do_id, fields))
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
				/*case DBSERVER_DELETE_STORED_OBJECT:
				{
					unsigned int verify = dgi.read_uint32();
					unsigned int do_id = dgi.read_uint32();

					m_log->spam() << "Deleting object with ID... " << do_id << std::endl;
					if(verify == 0x44696521)
					{
						m_db_engine->delete_object(do_id);
						m_freed_ids.push(do_id);
						m_log->spam() << "... delete successful." << std::endl;
					}
					else
					{
						m_log->spam() << "... delete failed." << std::endl;
					}
				}*/
				break;
				default:
					db_log.error() << "Recieved unknown MsgType: " << msg_type << std::endl;
			};
		}
};

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
