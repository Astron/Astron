#include "core/global.h"
#include "core/messages.h"
#include "dcparser/dcClass.h"
#include <exception>
#include <stdexcept>

#include "DistributedObject.h"
#include "StateServer.h"


ConfigVariable<channel_t> cfg_channel("control", 0);

StateServer::StateServer(RoleConfig roleconfig) : Role(roleconfig)
{
	channel_t channel = cfg_channel.get_rval(m_roleconfig);
	MessageDirector::singleton.subscribe_channel(this, channel);

	std::stringstream name;
	name << "StateServer(" << channel << ")";
	m_log = new LogCategory("stateserver", name.str());
}

StateServer::~StateServer()
{
	delete m_log;
}

void StateServer::handle_generate(DatagramIterator &dgi, bool has_other)
{
	unsigned int parent_id = dgi.read_uint32();
	unsigned int zone_id = dgi.read_uint32();
	unsigned short dc_id = dgi.read_uint16();
	unsigned int do_id = dgi.read_uint32();

	if(dc_id >= gDCF->get_num_classes())
	{
		m_log->error() << "Received create for unknown dclass ID=" << dc_id << std::endl;
		return;
	}

	if(m_objs.find(do_id) != m_objs.end())
	{
		m_log->warning() << "Received generate for already-existing object ID=" << do_id << std::endl;
		return;
	}

	DCClass *dclass = gDCF->get_class(dc_id);
	DistributedObject *obj;
	try
	{
		obj = new DistributedObject(this, do_id, dclass, parent_id, zone_id, dgi, has_other);
	}
	catch(std::exception &e)
	{
		m_log->error() << "Received truncated generate for "
		               << dclass->get_name() << "(" << do_id << ")" << std::endl;
		return;
	}
	m_objs[do_id] = obj;
}

void StateServer::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	unsigned short msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED:
		{
			handle_generate(dgi, false);
			break;
		}
		case STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER:
		{
			handle_generate(dgi, true);
			break;
		}
		case STATESERVER_SHARD_RESET:
		{
			channel_t ai_channel = dgi.read_uint64();
			std::set <channel_t> targets;
			for(auto it = m_objs.begin(); it != m_objs.end(); ++it)
				if(it->second && it->second->m_ai_channel == ai_channel && it->second->m_ai_explicitly_set)
					targets.insert(it->second->m_do_id);

			if(targets.size())
			{
				Datagram dg(targets, sender, STATESERVER_SHARD_RESET);
				dg.add_uint64(ai_channel);
				send(dg);
			}
			break;
		}
		default:
			m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
	}
}

RoleFactoryItem<StateServer> ss_fact("stateserver");
