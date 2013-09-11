#pragma once

#include "util/Role.h"
#include "core/RoleFactory.h"
#include "Database.h"
#include "DatabaseFactory.h"

class Database;

class DatabaseServer : public Role
{
friend class Database;
public:
	DatabaseServer(DatabaseConfig dbconfig);
	~DatabaseServer();

	virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);
private:
	Database *m_db;
	LogCategory *m_log;
	unsigned int m_freeId;
};
