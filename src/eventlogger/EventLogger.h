#pragma once

#include "core/global.h"
#include "util/Role.h"

class EventLogger : public Role
{
public:
	EventLogger(RoleConfig roleconfig);

	void handle_datagram(Datagram &in_dg, DatagramIterator &dgi) { } // Doesn't take DGs.

private:
	LogCategory m_log;
};
