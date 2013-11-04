#include "DBEngineFactory.h"

DBEngineFactory DBEngineFactory::singleton;

DBEngineFactory::DBEngineFactory()
{
}

IDatabaseEngine* DBEngineFactory::instantiate(const std::string &engine_name, DBEngineConfig config,
        doid_t min_id, doid_t max_id)
{
	if(m_creators[engine_name])
	{
		return m_creators[engine_name]->instantiate(config, min_id, max_id);
	}
	return NULL;
}

void DBEngineFactory::add_creator(const std::string &name, BaseDBEngineCreator* creator)
{
	m_creators[name] = creator;
}