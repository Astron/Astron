#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7198");

class ClientAgent : public Role
{
public:
	ClientAgent(RoleConfig roleconfig) : Role(roleconfig)
	{
		std::stringstream ss;
		ss << "Client Agent (" << bind_addr.get_rval(roleconfig) << ")";
		m_log = new LogCategory("clientagent", ss.str());
	}

	~ClientAgent()
	{
		delete m_log;
	}

	void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
	{

	}

private:
	LogCategory *m_log;
};

static RoleFactoryItem<ClientAgent> ca_fact("clientagent");
