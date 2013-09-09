#pragma once

#include "util/Role.h"
#include "core/RoleFactory.h"

class StateServer : public Role
{
public:
	StateServer(RoleConfig roleconfig);

	void handle_generate(DatagramIterator &dgi, bool has_other);

	virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

private:
	LogCategory *m_log;
};
