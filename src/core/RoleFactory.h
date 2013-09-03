#pragma once
#include "messagedirector/MessageDirector.h"
#include <hash_map>

class BaseRoleFactoryItem
{
	protected:
		BaseRoleFactoryItem(const std::string &name);
};

template<class T>
class RoleFactoryItem : public BaseRoleFactoryItem
{
	public:
		RoleFactoryItem(const std::string &name);
		T* instantiate();
};

class RoleFactory
{
	public:
		MDParticipantInterface* instantiate_role(const std::string &role_name);
		static RoleFactory singleton;

		void add_role(const std::string &name, BaseRoleFactoryItem *factory);
	private:
		std::hash_map<std::string, BaseRoleFactoryItem*> m_factories;
};