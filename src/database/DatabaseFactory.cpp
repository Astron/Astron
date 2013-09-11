#include "DatabaseFactory.h"

DatabaseFactory DatabaseFactory::singleton;

BaseDatabaseItem::BaseDatabaseItem(const std::string &name)
{
	DatabaseFactory::singleton.add_db(name, this);
}

void DatabaseFactory::add_db(const std::string &name, BaseDatabaseItem *factory)
{
	m_factories[name] = factory;
}

Database* DatabaseFactory::instantiate_db(const std::string &db_type, DatabaseConfig dbconfig)
{
	if(m_factories.find(db_type) != m_factories.end())
	{
		return m_factories[db_type]->instantiate(dbconfig);
	}
	return NULL;
}