#pragma once
#include "Database.h"
#include <unordered_map>

class BaseDatabaseItem {
public:
	virtual Database* instantiate(DatabaseConfig dbconfig) = 0;

protected:
	BaseDatabaseItem();
};

template<class T>
class DatabaseItem : public BaseDatabaseItem
{
public:
	virtual Database* instantiate(DatabaseConfig dbconfig) {
		return new T(dbconfig);
	}
};

class DatabaseFactory
{
public:
	Database* instantiate_db(std::string type, DatabaseConfig dbconfig);
	static DatabaseFactory singleton;

	void add_db(const std::string &name, BaseDatabaseItem *factory);
private:
	std::unordered_map<std::string, BaseDatabaseItem*> m_factories;
};
