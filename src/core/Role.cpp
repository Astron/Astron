#include "Role.h"
using namespace std;

static KeyedConfigList roles_config("roles", "type");

RoleConfigGroup::RoleConfigGroup(const string& type) : KeyedConfigGroup(type, roles_config)
{
}

// Constructor
Role::Role(RoleConfig roleconfig) : m_roleconfig(roleconfig)
{
}
