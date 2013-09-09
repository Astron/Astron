#pragma once

#include <map>
#include "util/Role.h"
#include "core/RoleFactory.h"

class DistributedObject;

class StateServer : public Role
{
friend class DistributedObject;

public:
	StateServer(RoleConfig roleconfig);

	void handle_generate(DatagramIterator &dgi, bool has_other);

	virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

private:
	LogCategory *m_log;
	std::map<unsigned int, DistributedObject*> m_objs;
};
