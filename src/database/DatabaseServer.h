#pragma once
#include <stack>
#include <vector>

#include "core/messages.h"
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
	LogCategory *m_log;

	channel_t m_channel;
	Database *m_db;

	unsigned int m_start_id;
	unsigned int m_end_id;

	unsigned int m_free_id;
	std::stack<unsigned int, std::vector<unsigned int>> m_freed_ids;
};
