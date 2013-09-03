#pragma once
#include "messagedirector/MessageDirector.h"
#include "core/config.h"

class Role : MDParticipantInterface
{
	protected:
		Role(RoleConfig roleconfig) : m_roleconfig(roleconfig)
		{
		}

		RoleConfig m_roleconfig;
};