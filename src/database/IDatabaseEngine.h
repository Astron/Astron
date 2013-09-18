#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"
#include <vector>

struct DatabaseObject
{
	unsigned short dc_id;
	std::map<DCField*, std::vector<unsigned char>> fields;

	DatabaseObject() {}
	DatabaseObject(unsigned short dcid) : dc_id(dcid) {}
};

class IDatabaseEngine
{
	public:
		IDatabaseEngine(DBEngineConfig dbeconfig, unsigned int min_id, unsigned int max_id) :
			m_config(dbeconfig), m_min_id(min_id), m_max_id(max_id) {}

		virtual unsigned int create_object(const DatabaseObject &dbo) = 0;
		virtual bool get_object(unsigned int do_id, DatabaseObject &dbo) = 0;
		virtual void delete_object(unsigned int do_id) = 0;
	protected:
		DBEngineConfig m_config;
		unsigned int m_min_id;
		unsigned int m_max_id;
};