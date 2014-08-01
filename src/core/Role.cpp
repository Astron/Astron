#include "Role.h"
using namespace std;

static KeyedConfigList roles_config("roles", "type");

RoleConfigGroup::RoleConfigGroup(const string& type) :
    ConfigGroup(type, roles_config), m_type("type", type, this)
{
}

// Constructor
Role::Role(RoleConfig roleconfig) : m_roleconfig(roleconfig)
{
}
