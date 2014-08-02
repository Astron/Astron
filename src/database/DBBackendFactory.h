#pragma once
#include "DatabaseBackend.h"
#include "core/types.h"
#include "config/ConfigVariable.h"
#include <string>
#include <cstdint>

// A BaseDBBackendFactoryItem is a common ancestor that all
//     DatabaseBackend factory templates inherit from.
class BaseDBBackendFactoryItem
{
  public:
    virtual DatabaseBackend* instantiate(ConfigNode config, doid_t min_id, doid_t max_id) = 0;
  protected:
    BaseDBBackendFactoryItem(const std::string &name);
};

// A DBBackendFactoryItem is the factory for a particular database backend.
// Each new role should declare a DBBackendFactoryItem<BackendClass>("BackendName");
template<class T>
class DBBackendFactoryItem : public BaseDBBackendFactoryItem
{
  public:
    DBBackendFactoryItem(const std::string& name) : BaseDBBackendFactoryItem(name)
    {
    }

    virtual DatabaseBackend* instantiate(ConfigNode config, doid_t min_id, doid_t max_id)
    {
        return new T(config, min_id, max_id);
    }
};

// The DBBackendFactory is a singleton that instantiates DatabaseBackends from a backend's name.
class DBBackendFactory
{
  public:
    static DBBackendFactory& singleton();

    // instantiate_backend creates a new DatabaseBackend object of type 'backend_name'.
    DatabaseBackend* instantiate_backend(const std::string &backend_name, ConfigNode config,
                                         doid_t min_id, doid_t max_id);


    // add_backend adds a factory for backend of type 'name'
    // It is called automatically when instantiating a new BaseDBBackendFactoryItem.
    void add_backend(const std::string &name, BaseDBBackendFactoryItem* factory);

    // has_backend returns true if a backend exists for 'name'.
    bool has_backend(const std::string &name);

  private:
    std::map<std::string, BaseDBBackendFactoryItem*> m_factories;
};
