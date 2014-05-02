#pragma once
#include "core/Role.h"
#include "core/RoleFactory.h"
#include "DatabaseBackend.h"

extern RoleConfigGroup dbserver_config;

class DBOperationImpl;
class DBOperationImpl_Create;
class DBOperationImpl_Delete;

class DatabaseServer : public Role
{
	public:
		DatabaseServer(RoleConfig);

		virtual void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);
		
	private:
		void handle_operation(DBOperationImpl *op);

		DatabaseBackend *m_db_backend;
		LogCategory *m_log;

		channel_t m_control_channel;
		doid_t m_min_id, m_max_id;
		bool m_broadcast;

		friend class DBOperationImpl;
		friend class DBOperationImpl_Create;
		friend class DBOperationImpl_Delete;
};
