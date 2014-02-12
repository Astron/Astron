#pragma once
#include "core/Role.h"
#include "core/RoleFactory.h"
#include "DatabaseBackend.h"

extern ConfigGroup db_backend_config;
extern ConfigVariable<std::string> db_backend_type;

class DatabaseServer : public Role
{
	public:
		DatabaseServer(RoleConfig);

		virtual void handle_datagram(Datagram_ptr &in_dg, DatagramIterator &dgi);
		
	private:
		DatabaseBackend *m_db_backend;
		LogCategory *m_log;

		channel_t m_control_channel;
		doid_t m_min_id, m_max_id;
};
