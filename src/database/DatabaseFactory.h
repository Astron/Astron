#pragma once
#include "Database.h"
#include "core/config.h"
#include <unordered_map>

class BaseDatabaseItem
{
public:
	virtual Database* instantiate(DatabaseConfig dbconfig) = 0;
protected:
	BaseDatabaseItem(const std::string &name);
};

template<class T>
class DatabaseItem : public BaseDatabaseItem
{
public:
	DatabaseItem(const std::string &name) : BaseDatabaseItem(name)
	{
	}

	virtual Database* instantiate(DatabaseConfig dbconfig)
	{
		return new T(dbconfig);
	}
};

class DatabaseFactory
{
public:
	Database* instantiate_db(const std::string &db_type, DatabaseConfig dbconfig);
	static DatabaseFactory singleton;

	void add_db(const std::string &name, BaseDatabaseItem *factory);
private:
	std::unordered_map<std::string, BaseDatabaseItem*> m_factories;
};
