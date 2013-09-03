#pragma once
#include "messagedirector/MessageDirector.h"
#include "util/Role.h"
#include <unordered_map>

class BaseRoleFactoryItem
{
	public:
		virtual Role* instantiate(RoleConfig roleconfig) = 0;
	protected:
		BaseRoleFactoryItem(const std::string &name);
};

template<class T>
class RoleFactoryItem : public BaseRoleFactoryItem
{
	public:
		RoleFactoryItem(const std::string &name) : BaseRoleFactoryItem(name)
		{
		}

		virtual Role* instantiate(RoleConfig roleconfig)
		{
			return new T(roleconfig);
		}
};

class RoleFactory
{
	public:
		Role* instantiate_role(const std::string &role_name, RoleConfig roleconfig);
		static RoleFactory singleton;

		void add_role(const std::string &name, BaseRoleFactoryItem *factory);
	private:
		std::unordered_map<std::string, BaseRoleFactoryItem*> m_factories;
};