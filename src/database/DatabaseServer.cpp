#include "core/global.h"
#include "core/messages.h"
#include <fstream>

#include "DatabaseServer.h"

ConfigVariable<channel_t> control_channel("control", 0);
ConfigVariable<unsigned int> id_start("generate/min", 1000);
ConfigVariable<unsigned int> id_end("generate/max", 2000);
ConfigVariable<std::string> storage_folder("storage_folder", "objs/");
LogCategory db_log("db", "Database");

void WriteIDFile(RoleConfig &roleconfig, unsigned int doId)
{
	std::ofstream file;
	std::stringstream ss;
	ss << storage_folder.get_rval(roleconfig)  << "id.txt";
	file.open(ss.str());
	file << doId;
	file.close();
}

DatabaseServer::DatabaseServer(RoleConfig roleconfig) : Role(roleconfig)
{
	m_freeId = id_start.get_rval(roleconfig);

	std::ifstream file;
	std::stringstream ss;
	ss << storage_folder.get_rval(roleconfig)  << "id.txt";
	file.open(ss.str());
	if(file.is_open())
	{
		file >> m_freeId;
	}
	file.close();

	subscribe_channel(control_channel.get_rval(roleconfig));
}

DatabaseServer::~DatabaseServer()
{}

void DatabaseServer::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	unsigned long long sender = dgi.read_uint64();
	unsigned short msg_type = dgi.read_uint16();
	switch(msg_type)
	{
		case DBSERVER_CREATE_STORED_OBJECT:
		{
			unsigned int context = dgi.read_uint32();
			unsigned short dcId = dgi.read_uint16();
			DCClass *dcc = gDCF->get_class(dcId);
			unsigned short nFields = dgi.read_uint16();
			unsigned int doId = m_freeId;
			if(doId > id_end.get_rval(m_roleconfig))
			{
				db_log.fatal() << "DB ran out of doIds" << std::endl;
				exit(1);
			}
			m_freeId++;
			WriteIDFile(m_roleconfig, m_freeId);

			Datagram serializedDO;
			serializedDO.add_uint32(doId);
			serializedDO.add_uint16(nFields);

			for(unsigned short  i = 0; i < nFields; ++i)
			{
				unsigned short fieldId = dgi.read_uint16();
				DCField *field = dcc->get_field(fieldId);
				if(field)
				{
					serializedDO.add_string(field->get_name());
					std::string data;
					dgi.unpack_field(field, data);
					serializedDO.add_string(data);
				}
			}

			std::stringstream ss;
			ss << storage_folder.get_rval(m_roleconfig)  << doId;
			std::ofstream file;
			file.open(ss.str());
			Datagram resp;
			resp.add_uint8(1);
			resp.add_uint64(sender);
			resp.add_uint64(control_channel.get_rval(m_roleconfig));
			resp.add_uint16(DBSERVER_CREATE_STORED_OBJECT_RESP);

			resp.add_uint32(context);
			if(file.is_open())
			{
				file.write(serializedDO.get_data(), serializedDO.get_buf_end());
				resp.add_uint32(doId);
			}
			else
			{
				resp.add_uint32(0);
			}
			file.close();
			send(resp);
		}
		break;
		default:
			db_log.error() << "DB recv'd unknown MsgType: " << msg_type
				<< std::endl;
	};
}

RoleFactoryItem<DatabaseServer> dbserver_fact("database");