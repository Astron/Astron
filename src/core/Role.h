#pragma once
#include "messagedirector/MessageDirector.h"
#include "core/config.h"

// A Role is a major component of Astron which is configured in the daemon's config file.
// Can send or receive datagram messages with the MessageDirector.
class Role : public MDParticipantInterface
{
	protected:
		Role(RoleConfig roleconfig) : m_roleconfig(roleconfig)
		{
		}

		RoleConfig m_roleconfig;
};
