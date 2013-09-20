#include "RoleFactory.h"

RoleFactory RoleFactory::singleton;

BaseRoleFactoryItem::BaseRoleFactoryItem(const std::string &name)
{
	RoleFactory::singleton.add_role(name, this);
}

void RoleFactory::add_role(const std::string &name, BaseRoleFactoryItem *factory)
{
	m_factories[name] = factory;
}

Role* RoleFactory::instantiate_role(const std::string &role_name, RoleConfig roleconfig)
{
	if(m_factories.find(role_name) != m_factories.end())
	{
		return m_factories[role_name]->instantiate(roleconfig);
	}
	return NULL;
}
