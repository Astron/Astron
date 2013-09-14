#include "DBEngineFactory.h"

DBEngineFactory DBEngineFactory::singleton;

DBEngineFactory::DBEngineFactory()
{
}

IDatabaseEngine* DBEngineFactory::instantiate(const std::string &engine_name, DBEngineConfig config, unsigned int start_do_id)
{
	if(m_creators[engine_name])
	{
		return m_creators[engine_name]->instantiate(config, start_do_id);
	}
	return NULL;
}