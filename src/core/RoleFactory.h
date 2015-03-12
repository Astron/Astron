#pragma once
#include "core/Role.h"
#include <unordered_map> // std::unordered_map

// A BaseRoleFactoryItem is a common ancestor that all role factory templates inherit from.
class BaseRoleFactoryItem
{
  public:
    virtual Role* instantiate(RoleConfig roleconfig) = 0;
  protected:
    BaseRoleFactoryItem(const std::string &name);
};

// A RoleFactoryItem is the factory for a particular role.
// Each new role should declare a RoleFactoryItem<RoleClass>("RoleName");
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

// The RoleFactory is a singleton that instantiates roles from a role's name.
class RoleFactory
{
  public:
    static RoleFactory& singleton();

    // instantiate_role creates a new Role object of type 'role_name'.
    Role* instantiate_role(const std::string &role_name, RoleConfig roleconfig);

    // add_role adds a factory for role of type 'name'
    // It is called automatically when instantiating a new RoleFactoryItem.
    void add_role(const std::string &name, BaseRoleFactoryItem *factory);
  private:
    std::unordered_map<std::string, BaseRoleFactoryItem*> m_factories;
};
