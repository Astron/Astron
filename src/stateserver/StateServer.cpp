#include "util/Role.h"
#include "core/RoleFactory.h"

class StateServer : public Role
{
	public:
		StateServer(RoleConfig roleconfig) : Role(roleconfig)
		{
		}

		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi)
		{
			return true;
		}
};

RoleFactoryItem<StateServer> ss_fact("stateserver");