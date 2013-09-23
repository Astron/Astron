#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"
#include <vector>

struct DatabaseObject
{
	uint16_t dc_id;
	std::map<DCField*, std::vector<uint8_t>> fields;

	DatabaseObject() {}
	DatabaseObject(uint16_t dcid) : dc_id(dcid) {}
};

class IDatabaseEngine
{
	public:
		IDatabaseEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
			m_config(dbeconfig), m_min_id(min_id), m_max_id(max_id) {}

		virtual uint32_t create_object(const DatabaseObject &dbo) = 0;
		virtual void delete_object(uint32_t do_id) = 0;

		virtual bool get_object(uint32_t do_id, DatabaseObject &dbo) = 0;
		virtual void set_fields(uint32_t do_id, DatabaseObject &dbo) = 0;

		virtual DCClass* get_dclass(uint32_t do_id) = 0;
	protected:
		DBEngineConfig m_config;
		uint32_t m_min_id;
		uint32_t m_max_id;
};