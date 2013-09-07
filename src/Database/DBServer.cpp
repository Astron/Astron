#include "util/Role.h"
#include "core/RoleFactory.h"
#include "core/global.h"
#include "core/messages.h"

ConfigVariable<channel_t> control_channel("control", 0);
LogCategory db_log("db", "Database");

class DBServer : public Role
{
	public:
		DBServer(RoleConfig roleconfig) : Role(roleconfig)
		{
			subscribe_channel(control_channel.get_rval(roleconfig));
		}
	private:
		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi)
		{
			unsigned short msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				default:
					db_log.error() << "DB recv'd unknown MsgType: " << msg_type
						<< std::endl;
			};
		}
};

RoleFactoryItem<DBServer> dbserver_fact("database");