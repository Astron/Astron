#include "core/global.h"

#include "DBStateServer.h"
#include "LoadingObject.h"

// RoleConfig
static ConfigVariable<channel_t> database_channel("control", INVALID_CHANNEL);

// RangesConfig
static ConfigVariable<uint32_t> range_min("min", INVALID_DO_ID);
static ConfigVariable<uint32_t> range_max("max", UINT32_MAX);

DBStateServer::DBStateServer(RoleConfig roleconfig) : StateServer(roleconfig),
	m_db_channel(database_channel.get_rval(m_roleconfig))
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
}

void DBStateServer::handle_activate(DatagramIterator &dgi, bool has_other)
{
	uint32_t do_id = dgi.read_uint32();
	uint32_t parent_id = dgi.read_uint32();
	uint32_t zone_id = dgi.read_uint32();

	// Check object is not already active
	if(m_objs.find(do_id) != m_objs.end() && m_loading.find(do_id) != m_loading.end())
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
		uint32_t dc_id = dgi.read_uint16();

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
			break;
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

RoleFactoryItem<DBStateServer> dbss_fact("dbss");
