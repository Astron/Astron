#include "DatabaseServer.h"

#include "core/global.h"
#include <fstream>

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> id_start("generate/min", 1000);
ConfigVariable<unsigned int> id_end("generate/max", 2000);
ConfigVariable<std::string> storage_type("storage/type", "yaml");
LogCategory db_log("db", "Database");

DatabaseServer::DatabaseServer(RoleConfig roleconfig) : Role(roleconfig)
{
	m_log = &db_log;
	m_channel = control_channel.get_rval(roleconfig);
	m_db = DatabaseFactory::singleton.instantiate_db(storage_type.get_rval(roleconfig), roleconfig["storage"]);
	if(!m_db) {
		m_log->fatal() << "No storage backend of type '" << storage_type.get_rval(roleconfig) << "' exists." << std::endl;
		exit(1);
	}
	m_start_id = m_free_id = id_start.get_rval(roleconfig);
	m_end_id = id_end.get_rval(roleconfig);

	subscribe_channel(m_channel);
}

DatabaseServer::~DatabaseServer() {}

void DatabaseServer::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	unsigned short msg_type = dgi.read_uint16();
	switch(msg_type)
	{
		case DBSERVER_CREATE_STORED_OBJECT:
		{
			unsigned int context = dgi.read_uint32();
			unsigned short dc_id = dgi.read_uint16();
			unsigned short field_count = dgi.read_uint16();

			unsigned int do_id = m_free_id;
			if(do_id > m_end_id)
			{
				if(m_freed_ids.empty()) {
					m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
					Datagram resp;
					resp.add_server_header(sender, m_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
					resp.add_uint32(context);
					resp.add_uint32(0);
					send(resp);
					return;
				}

				do_id = m_freed_ids.top();
				m_freed_ids.pop();
			} else {
				m_free_id++;
			}

			m_log->spam() << "Creating stored object with ID: " << do_id << " ..." << std::endl;

			m_db->create_object(do_id, dc_id, field_count, dgi);

			Datagram resp;
			resp.add_server_header(sender, m_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
			resp.add_uint32(context);
			resp.add_uint32(do_id);
			send(resp);
		}
		break;
		case DBSERVER_DELETE_STORED_OBJECT:
		{
			unsigned int verify = dgi.read_uint32();
			unsigned int do_id = dgi.read_uint32();

			m_log->spam() << "Deleting object with ID... " << do_id << std::endl;
			if(verify == 0x44696521) {
				m_db->delete_object(do_id);
				m_freed_ids.push(do_id);
				m_log->spam() << "... delete successful." << std::endl;
			} else {
				m_log->spam() << "... delete failed." << std::endl;
			}
		}
		break;
		default:
			m_log->error() << "Recieved unknown MsgType: " << msg_type
				<< std::endl;
	};
}

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
