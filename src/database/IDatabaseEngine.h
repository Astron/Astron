#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"
#include <vector>
#include <stdint.h>


struct DatabaseObject
{
	uint16_t dc_id;
	std::map<DCField*, std::vector<uint8_t>> fields;

	DatabaseObject()
	{
	}
	DatabaseObject(uint16_t dcid) : dc_id(dcid)
	{
	}
};

class IDatabaseEngine
{
	public:
		IDatabaseEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
			m_config(dbeconfig), m_min_id(min_id), m_max_id(max_id) {}

		virtual uint32_t create_object(const DatabaseObject &dbo) = 0;
		virtual void delete_object(uint32_t do_id) = 0;
		virtual bool get_object(uint32_t do_id, DatabaseObject &dbo) = 0;

		//virtual bool get_exists(uint32_t do_id) = 0;
		virtual DCClass* get_class(uint32_t do_id) = 0;

#define val_t std::vector<uint8_t>
#define map_t std::map<DCField*, std::vector<uint8_t>>
		virtual void del_field(uint32_t do_id, DCField* field) = 0;
		virtual void del_fields(uint32_t do_id, const std::vector<DCField*> &fields) = 0;

		virtual void set_field(uint32_t do_id, DCField* field, const val_t &value) = 0;
		virtual void set_fields(uint32_t do_id, const map_t &fields) = 0;

		// If not-equals/-empty, current are returned using value(s)
		virtual bool set_field_if_empty(uint32_t do_id, DCField* field, val_t &value) = 0;
		virtual bool set_field_if_equals(uint32_t do_id, DCField* field,
		                                 const val_t &equal, val_t &value) = 0;
		virtual bool set_fields_if_equals(uint32_t do_id, const map_t &equals, map_t &values) = 0;
		virtual bool get_field(uint32_t do_id, const DCField* field, val_t &value) = 0;
		virtual bool get_fields(uint32_t do_id,  const std::vector<DCField*> &fields, map_t &values) = 0;
#undef map_t
#undef val_t
	protected:
		DBEngineConfig m_config;
		uint32_t m_min_id;
		uint32_t m_max_id;
};