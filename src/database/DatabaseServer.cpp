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
			DCClass *dcc = gDCF->get_class(dc_id);
			unsigned short field_count = dgi.read_uint16();
			unsigned int do_id = m_free_id;
			if(do_id > m_end_id)
			{
				m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
				Datagram resp;
				resp.add_server_header(sender, m_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
				resp.add_uint32(context);
				resp.add_uint32(0);
				send(resp);
				return;
			}
			m_free_id++;

			m_log->spam() << "Created stored object with ID: " << do_id << std::endl;

			FieldList fields;
			for(unsigned short  i = 0; i < field_count; ++i)
			{
				unsigned short field_id = dgi.read_uint16();
				DCField *field = dcc->get_field(field_id);
				if(field && field->is_db())
				{
					fields.emplace(fields.end(), field, "");
					dgi.unpack_field(field, fields.back().second);
				}
			}

			m_db->create_object(do_id, fields);

			Datagram resp;
			resp.add_server_header(sender, m_channel, DBSERVER_CREATE_STORED_OBJECT_RESP);
			resp.add_uint32(context);
			resp.add_uint32(do_id);
			send(resp);
		}
		break;
		default:
			db_log.error() << "Recieved unknown MsgType: " << msg_type
				<< std::endl;
	};
}

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
