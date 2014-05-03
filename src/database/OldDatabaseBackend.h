#pragma once
#include "DatabaseBackend.h"
#include "core/types.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"
#include <vector>
#include <mutex>

struct ObjectData
{
	uint16_t dc_id;
	std::unordered_map<const dclass::Field*, std::vector<uint8_t> > fields;

	ObjectData()
	{
	}
	ObjectData(uint16_t dcid) : dc_id(dcid)
	{
	}
};

// This is an adaptor class from the new DBOperation-driven DatabaseBackend
// interface to the backends built on the old-style synchronous interface.
// It's largely temporary; once the other backends are moved to the asynchronous
// architecture, this will be removed.
class OldDatabaseBackend : public DatabaseBackend
{
	public:
		OldDatabaseBackend(ConfigNode dbeconfig, doid_t min_id, doid_t max_id) :
			DatabaseBackend(dbeconfig, min_id, max_id) {}

		virtual void submit(DBOperation *operation);

	protected:
		virtual doid_t create_object(const ObjectData &dbo) = 0;
		virtual void delete_object(doid_t do_id) = 0;
		virtual bool get_object(doid_t do_id, ObjectData &dbo) = 0;

		//virtual bool get_exists(uint32_t do_id) = 0;
		virtual const dclass::Class* get_class(doid_t do_id) = 0;

#define val_t std::vector<uint8_t>
#define map_t std::map<const dclass::Field*, std::vector<uint8_t> >
		virtual void del_field(doid_t do_id, const dclass::Field* field) = 0;
		virtual void del_fields(doid_t do_id, const std::vector<const dclass::Field*> &fields) = 0;

		virtual void set_field(doid_t do_id, const dclass::Field* field, const val_t &value) = 0;
		virtual void set_fields(doid_t do_id, const map_t &fields) = 0;

		// If not-equals/-empty, current are returned using value(s)
		virtual bool set_field_if_empty(doid_t do_id, const dclass::Field* field, val_t &value) = 0;
		virtual bool set_field_if_equals(doid_t do_id, const dclass::Field* field,
		                                 const val_t &equal, val_t &value) = 0;
		virtual bool set_fields_if_equals(doid_t do_id, const map_t &equals, map_t &values) = 0;
		virtual bool get_field(doid_t do_id, const dclass::Field* field, val_t &value) = 0;
		virtual bool get_fields(doid_t do_id, const std::vector<const dclass::Field*> &fields,
		                        map_t &values) = 0;
#undef map_t
#undef val_t

	private:
		std::mutex m_submit_lock;
};
