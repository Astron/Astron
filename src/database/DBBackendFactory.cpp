#include "DBBackendFactory.h"

BaseDBBackendFactoryItem::BaseDBBackendFactoryItem(const std::string &name)
{
    DBBackendFactory::singleton().add_backend(name, this);
}

DBBackendFactory& DBBackendFactory::singleton()
{
    static DBBackendFactory fact;
    return fact;
}

// instantiate_backend creates a new DatabaseBackend object of type 'backend_name'.
DatabaseBackend* DBBackendFactory::instantiate_backend(const std::string &backend_name,
        ConfigNode config, doid_t min_id, doid_t max_id)
{
    if(m_factories.find(backend_name) != m_factories.end()) {
        return m_factories[backend_name]->instantiate(config, min_id, max_id);
    }
    return nullptr;
}

// add_backend adds a factory for backend of type 'name'
// It is called automatically when instantiating a new BaseDBBackendFactoryItem.
void DBBackendFactory::add_backend(const std::string &name, BaseDBBackendFactoryItem* factory)
{
    m_factories[name] = factory;
}

// has_backend returns true if a backend exists for 'name'.
bool DBBackendFactory::has_backend(const std::string &name)
{
    return m_factories.find(name) != m_factories.end();
}
