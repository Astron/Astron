#include "StateServer.h"
#include "DistributedObject.h"

#include <vector>

// RoleConfig
static ConfigVariable<channel_t> database_channel("control", INVALID_CHANNEL);

// RangesConfig
static ConfigVariable<uint32_t> range_min("min", INVALID_DO_ID);
static ConfigVariable<uint32_t> range_max("max", UINT32_MAX);

class DBStateServer : public StateServer
{
	private:
		channel_t m_db_channel;
	public:
		DBStateServer(RoleConfig roleconfig) : StateServer(roleconfig),
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

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{
			channel_t sender = dgi.read_uint64();
			uint16_t msgtype = dgi.read_uint16();
			switch(msgtype)
			{
				case DBSS_OBJECT_ACTIVATE:
				{
					break;
				}
				case DBSS_OBJECT_DELETE_DISK:
				{
					break;
				}
			}
		}
};

RoleFactoryItem<DBStateServer> dbss_fact("dbss");
