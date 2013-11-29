#include "DBEngineFactory.h"

DBEngineFactory DBEngineFactory::singleton;

BaseDBEngineFactoryItem::BaseDBEngineFactoryItem(const std::string &name)
{
	DBEngineFactory::singleton.add_backend(name, this);
}

// instantiate_engine creates a new IDatabaseEngine object of type 'engine_name'.
IDatabaseEngine* DBEngineFactory::instantiate_engine(const std::string &engine_name,
	DBEngineConfig config, doid_t min_id, doid_t max_id)
{
	if(m_factories[engine_name])
	{
		return m_factories[engine_name]->instantiate(config, min_id, max_id);
	}
	return NULL;
}

// add_backend adds a factory for backend of type 'name'
// It is called automatically when instantiating a new BaseDBEngineFactoryItem.
void DBEngineFactory::add_backend(const std::string &name, BaseDBEngineFactoryItem* factory)
{
	m_factories[name] = factory;
}