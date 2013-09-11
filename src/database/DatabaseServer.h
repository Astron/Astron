#pragma once

#include "util/Role.h"
#include "core/RoleFactory.h"

class Database
{
public:
	virtual void createObject(/* TODO: Args */);
	virtual void deleteObject(/* TODO: Args */);
	virtual void selectObject(/* TODO: Args */);
	virtual void updateObject(/* TODO: Args */);
	virtual void updateObjectIfEquals(/* TODO: Args */);

	virtual void selectQuery(/* TODO: Args */);
	virtual void updateQuery(/* TODO: Args */);
	virtual void deleteQuery(/* TODO: Args */);
};

class DatabaseServer : public Role
{
friend class Database;
public:
	DatabaseServer(RoleConfig roleconfig);
	~DatabaseServer();

	virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);
private:

	//Database m_db;
	LogCategory *m_log;
	unsigned int m_freeId;
};
