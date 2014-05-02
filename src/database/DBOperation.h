#pragma once
#include <unordered_set>
#include <unordered_map>

#include "core/types.h"

#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"

// This represents a "snapshot" of a particular object. It is essentially just a
// dclass and a map of fields.
class DBObjectSnapshot
{
	public:
		dclass::Class *m_dclass;

		// Absense from this map indicates that the field is either not present
		// on the object, or was not requested by the corresponding DBOperation.
		std::unordered_map<const dclass::Field*, std::vector<uint8_t> > m_fields;
};

// DBOperation represents any single operation that can be asynchronously
// executed on the database backend.
class DBOperation
{
	public:
		enum OperationType
		{
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

			// MODIFY_FIELDS operations are for changing fields on an object.
			// This includes adding fields, deleting fields, changing fields,
			// and operations that require "if equals".
			// The changes are stored in the m_set_fields map, where NULL
			// represents a field deletion.
			// The m_criteria_fields map can be used to specify the state of
			// the "if empty" or "if equal" fields. Again, NULL represents absent.
			MODIFY_FIELDS
		};

		// What the frontend is requesting from the backend. See enum above for
		// explanation.
		OperationType m_type;

		// MUST be present for all operations except CREATE_OBJECT.
		doid_t m_doid;

		// MUST be present for CREATE_OBJECT.
		const dclass::Class *m_dclass;

		// The fields that the frontend is requesting. Only meaningful in the
		// case of GET_FIELDS operations.
		std::unordered_set<const dclass::Field *> m_get_fields;

		// The fields that the frontend wants us to change, and the fields that
		// must be equal (or absent) for the change to complete atomically.
		// Only meaningful in the case of MODIFY_FIELDS.
		std::unordered_map<const dclass::Field*, std::vector<uint8_t> > m_set_fields;
		std::unordered_map<const dclass::Field*, std::vector<uint8_t> > m_criteria_fields;

		// ***CALLBACK FUNCTIONS***
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

		// This is used in the case of MODIFY_FIELDS operations where the fields
		// in m_criteria_fields were NOT satisfied. The backend provides a snapshot
		// of the current values to be sent back to the client.
		// (Or it may include a *complete* snapshot and the frontend will filter.)
		// The frontend gets ownership of this pointer once it's passed.
		virtual void on_criteria_mismatch(DBObjectSnapshot *snapshot) { }

		// The on_complete callback is overloaded. When called without any arguments,
		// it means the delete or modify succeeded.
		virtual void on_complete() { }

		// This variant is used for successful create:
		virtual void on_complete(doid_t new_doid) { }

		// This variant is used for GET_* -- N.B. the ownership of the pointer
		// transfers to the frontend.
		virtual void on_complete(DBObjectSnapshot *snapshot) { }
};
