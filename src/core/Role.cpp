#include "Role.h"
using namespace std;

static ConfigList roles_config("roles", "type");

RoleConfigGroup::RoleConfigGroup(const string& type) : ConfigGroup(type, roles_config)
{
	roles_config.add_element_group(this);
}

// Constructor
Role::Role(RoleConfig roleconfig) : m_roleconfig(roleconfig)
{
}
