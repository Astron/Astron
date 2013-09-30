#include "DistributedObject.h"
#include "StateServer.h"

static ConfigVariable<channel_t> database_channel("control", INVALID_CHANNEL);

class DBStateServer : public StateServer
{
	private:
		channel_t m_db_channel;
	public:
		DBStateServer(RoleConfig roleconfig) : StateServer(roleconfig)
		{
			m_db_channel = database_channel.get_rval(m_roleconfig);
		}
		~DBStateServer()
		{

		}

		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{

		}
};
