#pragma once
#include "DatabaseBackend.h"
#include "core/types.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"
#include <vector>
#include <mutex>

typedef std::vector<uint8_t> FieldValue;
typedef std::vector<const dclass::Field*> FieldList;

struct ObjectData {
    uint16_t dc_id;
    FieldValues fields;

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

    virtual void del_field(doid_t do_id, const dclass::Field* field) = 0;
    virtual void del_fields(doid_t do_id, const FieldList &fields) = 0;

    virtual void set_field(doid_t do_id, const dclass::Field* field,
                           const std::vector<uint8_t> &value) = 0;
    virtual void set_fields(doid_t do_id, const FieldValues &fields) = 0;

    // If not-equals/-empty, current are returned using value(s)
    virtual bool set_field_if_empty(doid_t do_id, const dclass::Field* field,
                                    std::vector<uint8_t> &value) = 0;
    virtual bool set_field_if_equals(doid_t do_id, const dclass::Field* field,
                                     const std::vector<uint8_t> &equal,
                                     std::vector<uint8_t> &value) = 0;
    virtual bool set_fields_if_equals(doid_t do_id, const FieldValues &equals,
                                      FieldValues &values) = 0;
    virtual bool get_field(doid_t do_id, const dclass::Field* field,
                           std::vector<uint8_t> &value) = 0;
    virtual bool get_fields(doid_t do_id, const FieldList &fields,
                            FieldValues &values) = 0;

  private:
    std::mutex m_submit_lock;
};
