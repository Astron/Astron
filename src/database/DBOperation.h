#pragma once
#include <set>
#include <map>

#include "core/types.h"
#include "core/objtypes.h"
#include "util/DatagramIterator.h"

// Foward declarations
class DatabaseServer;

// This represents a "snapshot" of a particular object. It is essentially just a
// dclass and a map of fields.
class DBObjectSnapshot
{
  public:
    const dclass::Class *m_dclass;

    // Absense from this map indicates that the field is either not present
    // on the object, or was not requested by the corresponding DBOperation.
    FieldValues m_fields;
};

// DBOperation represents any single operation that can be asynchronously
// executed on the database backend.
class DBOperation
{
  public:
    DBOperation(DatabaseServer *db) : m_dbserver(db) { }
    virtual ~DBOperation() { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi) = 0;

    enum OperationType {
        // CREATE_OBJECT operations create a new object on the database.
        // These require that the class be specified, but do not require a doid.
        CREATE_OBJECT,

        // DELETE_OBJECT operations delete an object from the database.
        // These require a doid.
        DELETE_OBJECT,

        // GET_OBJECT operations return a snapshot (dclass + fields) of an object.
        // These require a doid.
        GET_OBJECT,

        // GET_FIELDS operations only require certain fields.
        // Some backends may choose to treat this the same as GET_OBJECT and
        // return an entire snapshot instead of only the required fields
        // (the frontend will filter the response accordingly), while other
        // backends may run more efficiently knowing only the fields they
        // need to retrieve.
        // These require a doid and m_get_fields.
        GET_FIELDS,

        // SET_FIELDS operations are for setting (replaces existiing) fields on an object.
        // This includes adding unset fields, deleting set fields, and changing existing fields.
        // The changes are in the m_set_fields map, where nullptr represents a field deletion.
        SET_FIELDS,

        // UPDATE_FIELDS operations are like set fields operations, but with conditions
        // typically to verify that the object is in a sane state before applying updates.
        // The m_criteria_fields map can be used to specify the state of
        // the "if empty" or "if equal" fields where nullptr represents empty/absent.
        UPDATE_FIELDS
    };

    // === ATTRIBUTE ACCESSORS ===

    // type returns the OperationType for this operation. See enum above for explanation.
    OperationType type() const
    {
        return m_type;
    }

    // doid returns the id of the object being manipulated [except: CREATE_OBJECT]
    doid_t doid() const
    {
        return m_doid;
    }

    // dclass returns the class for a create object operation [only: CREATE_OBJECT]
    const dclass::Class *dclass() const
    {
        return m_dclass;
    }

    // get_fields returns the fields that the frontend is requesting [only: GET_FIELDS]
    const FieldSet& get_fields() const
    {
        return m_get_fields;
    }

    // set_fields returns the fields that the frontend wants us to change.
    // Used with CREATE_OBJECT, SET_FIELDS, UPDATE_FIELDS
    const FieldValues& set_fields() const
    {
        return m_set_fields;
    }

    // criteria_fields must be equal (or absent) for the change to complete atomically.
    // Only meaningful in the case of UPDATE_FIELDS.
    const FieldValues& criteria_fields() const
    {
        return m_criteria_fields;
    }

    // === CONVENIENCE FUNCTIONS ===

    // Because many operations that operate on existing objects are created
    // without foreknowledge of the object's dclass, the frontend cannot
    // double-check that the fields included in the request are appropriate
    // for that kind of object.
    //
    // If (when) the backend discovers the object's dclass, it should call
    // this function, which will sanity-check the operation to make sure it
    // is appropriate for the dclass. If this function returns true, then all
    // is well and the backend may proceed. If this function returns false,
    // then the backend is to abort the current operation (safely reverting
    // any changes it may already have made) and then call on_failure().
    //
    // This function handles logging, so the backend need not report when
    // false is returned.
    virtual bool verify_class(const dclass::Class *)
    {
        return true;
    }

    // This function determines whether two DBOperations are "independent" --
    // in other words, the outcome of one operation does not affect the other,
    // which makes their ordering unimportant. This is useful for concurrency -
    // if two operations are independent, it's safe to execute them in
    // parallel.
    //
    // This function MUST ONLY return true if the lack of ordering is safe,
    // but may return false in cases of uncertainty (i.e. two operations that
    // are actually independent may report that they are *not* independent
    // because it still would not be safe to execute them out of order).
    //
    // Note that this relation must also be symmetric. In other words,
    // a->is_independent_of(b) == b->is_independent_of(a)
    virtual bool is_independent_of(const DBOperation *other) const
    {
        // By default, we only assume operations are independent if they
        // operate on different doIds. CREATEs are assumed never independent -
        // their ordering is very much important - but their doid() is invalid
        // and invalid != invalid is false.
        return doid() != other->doid();
    }

    // === CALLBACK FUNCTION ===
    // The database backend invokes these when the operation completes.
    // N.B. when this happens, the DBOperation regains control of its own
    // pointer (therefore the backend should not perform any more operations
    // on the DBOperation* after one of these is called)

    // This is used to indicate a generic failure, it could mean:
    // * The database backend is broken.
    // * The requested object is absent.
    // * The frontend requested modification of a field that is not valid
    //   for the class.
    virtual void on_failure() { }

    // This is used in the case of UPDATE_FIELDS operations where the fields
    // in m_criteria_fields were NOT satisfied. The backend provides a snapshot
    // of the current values to be sent back to the client.
    // (Or it may include a *complete* snapshot and the frontend will filter.)
    // The frontend gets ownership of this pointer once it's passed.
    virtual void on_criteria_mismatch(DBObjectSnapshot *) { }

    // The on_complete callback is overloaded. When called without any arguments,
    // it means the delete or modify succeeded.
    virtual void on_complete() { }

    // This variant is used for successful create:
    virtual void on_complete(doid_t) { }

    // This variant is used for GET_* -- N.B. the ownership of the pointer
    // transfers to the frontend.
    virtual void on_complete(DBObjectSnapshot *) { }


  protected:
    // The DBServer the operation is acting on.
    DatabaseServer *m_dbserver;
    // The sender of the operation.
    channel_t m_sender;

    // m_type sets what the frontend is requesting from the backend. See OperationType enum.
    OperationType m_type;
    // m_doid MUST be present for all operations except CREATE_OBJECT.
    doid_t m_doid;
    // m_dclass MUST be present for CREATE_OBJECT.
    const dclass::Class *m_dclass;
    // The fields that the frontend is requesting. Only used in GET_FIELDS operations.
    FieldSet m_get_fields;
    // The fields that the frontend wants us to change.
    FieldValues m_set_fields;
    // The fields that must be equal (or absent) for the change to complete atomically.
    FieldValues m_criteria_fields;

    void cleanup();
    bool verify_fields(const dclass::Class *dclass, const FieldSet& fields);
    bool verify_fields(const dclass::Class *dclass, const FieldValues& fields);
    void announce_fields(const FieldValues& fields);
    bool populate_set_fields(DatagramIterator &dgi, uint16_t field_count,
                             bool deletes = false, bool values = false);
    bool populate_get_fields(DatagramIterator &dgi, uint16_t field_count);
};

class DBOperationCreate : public DBOperation
{
  public:
    DBOperationCreate(DatabaseServer *db) : DBOperation(db) { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi);
    virtual void on_complete(doid_t doid);
    virtual void on_failure();

  private:
    uint32_t m_context;
};
class DBOperationDelete : public DBOperation
{
  public:
    DBOperationDelete(DatabaseServer *db) : DBOperation(db) { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi);
    virtual void on_complete();
    virtual void on_failure();
};
class DBOperationGet : public DBOperation
{
  public:
    DBOperationGet(DatabaseServer *db) : DBOperation(db) { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi);
    virtual bool verify_class(const dclass::Class *dclass);
    virtual bool is_independent_of(const DBOperation *other) const;
    virtual void on_complete(DBObjectSnapshot *snapshot);
    virtual void on_failure();

  private:
    uint32_t m_context;
    uint16_t m_resp_msgtype;
};
class DBOperationSet : public DBOperation
{
  public:
    DBOperationSet(DatabaseServer *db) : DBOperation(db) { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi);
    virtual bool verify_class(const dclass::Class *dclass);
    virtual bool is_independent_of(const DBOperation *other) const;
    virtual void on_complete();
    virtual void on_failure();
};
class DBOperationUpdate : public DBOperation
{
  public:
    DBOperationUpdate(DatabaseServer *db) : DBOperation(db) { }
    virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi);
    virtual bool verify_class(const dclass::Class *dclass);
    virtual bool is_independent_of(const DBOperation *other) const;
    virtual void on_complete();
    virtual void on_failure();
    virtual void on_criteria_mismatch(DBObjectSnapshot *);

  private:
    uint32_t m_context;
    uint16_t m_resp_msgtype;
};
