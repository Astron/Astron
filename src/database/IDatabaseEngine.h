#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"

struct DatabaseObject
{
	unsigned int do_id;
	unsigned short dc_id;
	std::map<DCField*, std::string> fields;
};

class IDatabaseEngine
{
	public:
		IDatabaseEngine(DBEngineConfig dbeconfig, unsigned int start_id) : m_dbeconfig(dbeconfig),
			m_start_id(start_id)
		{
		}

		virtual unsigned int get_next_id() = 0;
		virtual bool create_object(const DatabaseObject &dbo) = 0;
		virtual bool get_object(DatabaseObject &dbo) = 0;
		virtual void delete_object(unsigned int do_id) = 0;
	protected:
		DBEngineConfig m_dbeconfig;
		unsigned int m_start_id;
};