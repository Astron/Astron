#include "DBOperation.h"
#include <vector>

#include "core/global.h"
#include "core/msgtypes.h"
#include "DatabaseServer.h"
using namespace std;
using dclass::Field;
using dclass::Class;

void DBOperation::cleanup()
{
    m_dbserver->clear_operation(this);

    delete this;
}

void DBOperation::announce_fields(const FieldValues& fields)
{
    // Calculate the fields that we are sending in our response:
    FieldValues changed_fields;
    FieldSet deleted_fields;
    for(auto it = fields.begin(); it != fields.end(); ++it) {
        if(it->second.empty())
            deleted_fields.insert(it->first);
        else
            changed_fields[it->first] = it->second;
    }

    // Send delete fields broadcast
    if(!deleted_fields.empty()) {
        bool multi = (deleted_fields.size() > 1);
        DatagramPtr update = Datagram::create();
        update->add_server_header(database_to_object(m_doid), m_sender,
                                  multi ? DBSERVER_OBJECT_DELETE_FIELDS :
                                  DBSERVER_OBJECT_DELETE_FIELD);
        update->add_doid(m_doid);
        if(multi) {
            update->add_uint16(deleted_fields.size());
        }
        for(auto it = deleted_fields.begin(); it != deleted_fields.end(); ++it) {
            update->add_uint16((*it)->get_id());
        }
        m_dbserver->route_datagram(update);
    }

    // Send update fields broadcast
    if(!changed_fields.empty()) {
        bool multi = (changed_fields.size() > 1);
        DatagramPtr update = Datagram::create();
        update->add_server_header(database_to_object(m_doid), m_sender,
                                  multi ? DBSERVER_OBJECT_SET_FIELDS :
                                  DBSERVER_OBJECT_SET_FIELD);
        update->add_doid(m_doid);
        if(multi) {
            update->add_uint16(changed_fields.size());
        }
        for(auto it = changed_fields.begin(); it != changed_fields.end(); ++it) {
            update->add_uint16(it->first->get_id());
            update->add_data(it->second);
        }
        m_dbserver->route_datagram(update);
    }
}
bool DBOperation::verify_fields(const dclass::Class *dclass, const FieldSet& fields)
{
    bool valid = true;
    for(auto it = fields.begin(); it != fields.end(); ++it) {
        const dclass::Field *field = *it;
        if(!dclass->get_field_by_id(field->get_id())) {
            m_dbserver->m_log->warning() << "Field " << field->get_name()
                                         << " does not belong to object " << m_doid
                                         << "(" << dclass->get_name() << ")\n";
            valid = false;
        }
    }
    return valid;
}

bool DBOperation::verify_fields(const dclass::Class *dclass, const FieldValues& fields)
{
    bool valid = true;
    for(auto it = fields.begin(); it != fields.end(); ++it) {
        if(!dclass->get_field_by_id(it->first->get_id())) {
            m_dbserver->m_log->warning() << "Field " << it->first->get_name()
                                         << " does not belong to object " << m_doid
                                         << "(" << dclass->get_name() << ")\n";
            valid = false;
        }
    }
    return valid;
}

bool DBOperation::populate_set_fields(DatagramIterator &dgi, uint16_t field_count,
                                      bool delete_values, bool check_values)
{
    for(uint16_t i = 0; i < field_count; ++i) {
        uint16_t field_id = dgi.read_uint16();
        const Field *field = g_dcf->get_field_by_id(field_id);
        if(!field) {
            m_dbserver->m_log->error() << "Create/modify field request included invalid field #"
                                       << field_id << "\n";
            return false;
        }

        if(field->has_keyword("db")) {
            // Get criteria value
            if(check_values) {
                dgi.unpack_field(field, m_criteria_fields[field]);
            }

            // Get update value
            if(!delete_values) {
                dgi.unpack_field(field, m_set_fields[field]);
            } else if(field->has_default_value()) {
                string val = field->get_default_value();
                m_set_fields[field] = vector<uint8_t>(val.begin(), val.end());
            } else {
                m_set_fields[field]; // Force insertion of blank vector
            }
        } else {
            m_dbserver->m_log->warning() << "Create/modify field request included non-DB field "
                                         << field->get_name() << "\n";

            // Don't read in a non-db field
            if(!delete_values) {
                dgi.skip_field(field);
            }

            // It is not proper to expect a non-db field in criteria.
            if(check_values) {
                return false;
            }
        }
    }

    // A MODIFY_FIELDS request is only valid if we're actually trying to
    // change fields. A CREATE_OBJECT request is valid even without fields.
    return (m_type == CREATE_OBJECT) || !m_set_fields.empty();
}

bool DBOperation::populate_get_fields(DatagramIterator &dgi, uint16_t field_count)
{
    for(uint16_t i = 0; i < field_count; ++i) {
        // Read the field from the datagram
        uint16_t field_id = dgi.read_uint16();
        const Field *field = g_dcf->get_field_by_id(field_id);
        if(!field) {
            m_dbserver->m_log->error() << "Get field request included invalid field #"
                                       << field_id << "\n";
            return false;
        }

        // Add the field to the fields we want to get from the database
        if(field->has_keyword("db"))
            m_get_fields.insert(field);
        else
            m_dbserver->m_log->error() << "Get field request included non-DB field "
                                       << field->get_name() << "\n";
    }

    // A GET_FIELDS request is only really valid if we're actually requesting fields.
    return !m_get_fields.empty();
}

bool DBOperationCreate::initialize(channel_t sender, uint16_t, DatagramIterator &dgi)
{
    m_sender = sender;

    m_type = CREATE_OBJECT;
    m_context = dgi.read_uint32();

    uint16_t dclass_id = dgi.read_uint16();
    m_dclass = g_dcf->get_class_by_id(dclass_id);
    if(!m_dclass) {
        m_dbserver->m_log->error() << "Create object request for invalid dclass ID #"
                                   << dclass_id << "\n";
        on_failure();
        return false;
    }

    uint16_t field_count = dgi.read_uint16();
    if(!populate_set_fields(dgi, field_count)) {
        on_failure();
        return false;
    }

    // Make sure that all present fields actually belong to the dclass.
    bool errors = false;
    for(auto it = m_set_fields.begin(); it != m_set_fields.end(); ++it) {
        if(!m_dclass->get_field_by_id(it->first->get_id())) {
            m_dbserver->m_log->warning() << "Attempted to create object "
                                         << m_dclass->get_name()
                                         << " which includes non-belonging field: "
                                         << it->first->get_name() << "\n";
            errors = true;
        }
    }
    if(errors) {
        on_failure();
        return false;
    }

    // Set all non-present fields to defaults (if they exist)
    for(unsigned int i = 0; i < m_dclass->get_num_fields(); ++i) {
        const dclass::Field *field = m_dclass->get_field(i);
        if(field->has_default_value() && field->has_keyword("db")
           && m_set_fields.find(field) == m_set_fields.end()) {
            string val = field->get_default_value();
            m_set_fields[field] = vector<uint8_t>(val.begin(), val.end());
        }
    }

    return true;
}

void DBOperationCreate::on_complete(doid_t doid)
{
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            DBSERVER_CREATE_OBJECT_RESP);
    resp->add_uint32(m_context);
    resp->add_doid(doid);
    m_dbserver->route_datagram(resp);

    cleanup();
}

void DBOperationCreate::on_failure()
{
    // We just send back an invalid response...
    on_complete(INVALID_DO_ID);
}

bool DBOperationDelete::initialize(channel_t sender, uint16_t, DatagramIterator &dgi)
{
    m_sender = sender;

    m_type = DELETE_OBJECT;
    m_doid = dgi.read_doid();

    return true;
}

void DBOperationDelete::on_failure()
{
    cleanup();
}

void DBOperationDelete::on_complete()
{
    // Broadcast update to object's channel
    if(m_dbserver->m_broadcast) {
        DatagramPtr update = Datagram::create();
        update->add_server_header(database_to_object(m_doid), m_sender, DBSERVER_OBJECT_DELETE);
        update->add_doid(m_doid);
        m_dbserver->route_datagram(update);
    }
    cleanup();
}

bool DBOperationGet::initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
{
    m_sender = sender;

    m_context = dgi.read_uint32();
    m_doid = dgi.read_doid();

    if(msg_type == DBSERVER_OBJECT_GET_ALL) {
        m_type = GET_OBJECT;
        m_resp_msgtype = DBSERVER_OBJECT_GET_ALL_RESP;
        return true;
    }

    m_type = GET_FIELDS;

    uint16_t field_count = 1;
    if(msg_type == DBSERVER_OBJECT_GET_FIELD) {
        m_resp_msgtype = DBSERVER_OBJECT_GET_FIELD_RESP;
    } else if(msg_type == DBSERVER_OBJECT_GET_FIELDS) {
        field_count = dgi.read_uint16();
        m_resp_msgtype = DBSERVER_OBJECT_GET_FIELDS_RESP;
    }

    return populate_get_fields(dgi, field_count);
}

bool DBOperationGet::verify_class(const dclass::Class *dclass)
{
    // If request is of type GET_OBJECT don't expect a class, so no need to verify it.
    if(m_type == GET_OBJECT) {
        return true;
    }

    // Otherwise verify the fields and spawn a warning for developers if its not valid
    else if(!verify_fields(dclass, m_get_fields)) {
        m_dbserver->m_log->warning() << "Request invalid field for " << m_doid
                                     << "(" << dclass->get_name() << ")\n";
    }

    // Though we may have encountered problems above, they aren't strictly
    // cause for failure. The fields will simply be absent from the reply.
    return true;
}

bool DBOperationGet::is_independent_of(const DBOperation *other) const
{
    if(doid() != other->doid()) {
        // Not even operating on the same doId; we're good.
        return true;
    }

    switch(other->type()) {
    case GET_OBJECT:
    case GET_FIELDS:
        return true;

    case SET_FIELDS:
    case UPDATE_FIELDS:
        if(type() == GET_OBJECT) {
            // A GET_OBJECT gets everything, and is therefore not
            // independent of any changes to the object.
            return false;
        }

        // Test if any of the fields we're getting is a field changed by
        // the other operation...
        for(auto it = get_fields().begin(); it != get_fields().end(); ++it) {
            if(other->set_fields().find(*it) != other->set_fields().end()) {
                return false;
            }
        }

        // No conflict.
        return true;

    default:
        return false;
    }
}

void DBOperationGet::on_failure()
{
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            m_resp_msgtype);
    resp->add_uint32(m_context);
    resp->add_uint8(FAILURE);
    m_dbserver->route_datagram(resp);

    cleanup();
}

void DBOperationGet::on_complete(DBObjectSnapshot *snapshot)
{
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            m_resp_msgtype);
    resp->add_uint32(m_context);
    resp->add_uint8(SUCCESS);

    // Calculate the fields that we are sending in our response:
    FieldValues response_fields;
    if(m_resp_msgtype == DBSERVER_OBJECT_GET_ALL_RESP) {
        // Send everything:
        response_fields = snapshot->m_fields;
    } else {
        // Send only what was requested:
        for(auto it = m_get_fields.begin(); it != m_get_fields.end(); ++it) {
            auto it2 = snapshot->m_fields.find(*it);
            if(it2 != snapshot->m_fields.end()) {
                response_fields[it2->first] = it2->second;
            }
        }
    }

    // WHAT we send depends on our m_resp_msgtype, so:
    if(m_resp_msgtype == DBSERVER_OBJECT_GET_FIELD_RESP) {
        if(response_fields.empty()) {
            // We did not find the field we were looking for.
            // Therefore, this is a failure.
            on_failure();
            return;
        }

        resp->add_uint16(response_fields.begin()->first->get_id());
        resp->add_data(response_fields.begin()->second);

        // And that's it. We're done.
        m_dbserver->route_datagram(resp);
        delete snapshot;
        cleanup();
        return;
    }

    if(m_resp_msgtype == DBSERVER_OBJECT_GET_ALL_RESP) {
        resp->add_uint16(snapshot->m_dclass->get_id());
    }

    resp->add_uint16(response_fields.size());
    for(auto it = response_fields.begin(); it != response_fields.end(); ++it) {
        resp->add_uint16(it->first->get_id());
        resp->add_data(it->second);
    }

    m_dbserver->route_datagram(resp);
    delete snapshot;
    cleanup();
}

bool DBOperationSet::initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
{
    m_sender = sender;
    m_type = SET_FIELDS;
    m_doid = dgi.read_doid();

    uint16_t field_count;
    if(msg_type == DBSERVER_OBJECT_SET_FIELDS ||
       msg_type == DBSERVER_OBJECT_DELETE_FIELDS) {
        field_count = dgi.read_uint16();
    } else {
        field_count = 1;
    }

    bool delete_values = false;
    if(msg_type == DBSERVER_OBJECT_DELETE_FIELD ||
       msg_type == DBSERVER_OBJECT_DELETE_FIELDS) {
        delete_values = true;
    }

    if(!populate_set_fields(dgi, field_count, delete_values, false)) {
        on_failure();
        return false;
    }

    return true;
}

bool DBOperationSet::verify_class(const dclass::Class *dclass)
{
    if(!verify_fields(dclass, m_set_fields)) {
        m_dbserver->m_log->warning() << "Attempted to modify invalid field for " << m_doid
                                     << "(" << dclass->get_name() << ")\n";
        return false;
    }
    return true;
}

bool DBOperationSet::is_independent_of(const DBOperation *other) const
{
    if(doid() != other->doid()) {
        // Not even operating on the same doId; we're good.
        return true;
    }

    switch(other->type()) {
    case GET_OBJECT:
    case GET_FIELDS:
        // The logic for this is in DBOperationGet... Let's let that take
        // care of it.
        return other->is_independent_of(this);

    case SET_FIELDS:
        // The operations are independent as long as they're on different
        // fields.

        for(auto it = set_fields().begin(); it != set_fields().end(); ++it) {
            if(other->set_fields().find(it->first) != other->set_fields().end()) {
                return false;
            }
        }

        // No conflict.
        return true;

    case UPDATE_FIELDS:
        // We must be independent of the other operation's fields *AND*
        // their criteria.
        for(auto it = set_fields().begin(); it != set_fields().end(); ++it) {
            if(other->set_fields().find(it->first) != other->set_fields().end() ||
               other->criteria_fields().find(it->first) != other->criteria_fields().end()) {
                return false;
            }
        }

        // No conflict.
        return true;

    default:
        return false;
    }
}

void DBOperationSet::on_complete()
{
    // Broadcast update to object's channel
    if(m_dbserver->m_broadcast) {
        announce_fields(m_set_fields);
    }

    cleanup();
}

void DBOperationSet::on_failure()
{
    cleanup();
}

bool DBOperationUpdate::initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
{
    m_sender = sender;
    m_type = UPDATE_FIELDS;
    m_context = dgi.read_uint32();
    m_doid = dgi.read_doid();

    uint16_t field_count;
    if(msg_type == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS) {
        field_count = dgi.read_uint16();
    } else {
        field_count = 1;
    }

    bool check_values = true;
    if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EQUALS) {
        m_resp_msgtype = DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP;
    } else if(msg_type == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS) {
        m_resp_msgtype = DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP;
    } else if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EMPTY) {
        m_resp_msgtype = DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP;
        check_values = false;
    }

    if(!populate_set_fields(dgi, field_count, false, check_values)) {
        on_failure();
        return false;
    }

    if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EMPTY) {
        // We must satisfy the IF_EMPTY constraint here, because we don't
        // handle it in populate_set_fields.
        m_criteria_fields[m_set_fields.begin()->first]; // Force insertion of blank vector
    }

    return true;
}

bool DBOperationUpdate::verify_class(const dclass::Class *dclass)
{
    bool errors = false;
    if(!verify_fields(dclass, m_set_fields)) {
        m_dbserver->m_log->warning() << "Attempted to modify invalid field for " << m_doid
                                     << "(" << dclass->get_name() << ")\n";
        errors = true;
    }
    if(!verify_fields(dclass, m_criteria_fields)) {
        m_dbserver->m_log->warning() << "Received invalid criterion field for " << m_doid
                                     << "(" << dclass->get_name() << ")\n";
        errors = true;
    }
    return !errors;
}

bool DBOperationUpdate::is_independent_of(const DBOperation *other) const
{
    if(doid() != other->doid()) {
        // Not even operating on the same doId; we're good.
        return true;
    }

    switch(other->type()) {
    case GET_OBJECT:
    case GET_FIELDS:
    case SET_FIELDS:
        // The logic for handling these types is elsewhere.
        return other->is_independent_of(this);

    case UPDATE_FIELDS:
        // This case gets really tricky. Not only may our set fields not
        // conflict with their set fields/criteria, but our *criteria* may
        // not overlap with their set fields. It *is* okay, however, to
        // have a criteria overlap.
        for(auto it = set_fields().begin(); it != set_fields().end(); ++it) {
            if(other->set_fields().find(it->first) != other->set_fields().end() ||
               other->criteria_fields().find(it->first) != other->criteria_fields().end()) {
                return false;
            }
        }

        // Our set fields don't conflict, let's see if their set fields
        // conflict.
        for(auto it = other->set_fields().begin(); it != other->set_fields().end(); ++it) {
            // We're only testing against our criteria_fields()... The
            // set_fields()<->set_fields() intersection was already tested.
            if(criteria_fields().find(it->first) != criteria_fields().end()) {
                return false;
            }
        }

        // No conflict.
        return true;

    default:
        return false;
    }
}

void DBOperationUpdate::on_complete()
{
    // Broadcast update to object's channel
    if(m_dbserver->m_broadcast) {
        announce_fields(m_set_fields);
    }

    // Send update response
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            m_resp_msgtype);
    resp->add_uint32(m_context);
    resp->add_uint8(SUCCESS);
    m_dbserver->route_datagram(resp);

    cleanup();
}

void DBOperationUpdate::on_failure()
{
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            m_resp_msgtype);
    resp->add_uint32(m_context);
    resp->add_uint8(FAILURE);
    m_dbserver->route_datagram(resp);

    cleanup();
}

void DBOperationUpdate::on_criteria_mismatch(DBObjectSnapshot *snapshot)
{
    DatagramPtr resp = Datagram::create();
    resp->add_server_header(m_sender, m_dbserver->m_control_channel,
                            m_resp_msgtype);
    resp->add_uint32(m_context);
    resp->add_uint8(FAILURE);

    // Calculate the fields that we are sending in our response:
    FieldValues mismatched_fields;

    for(auto it = m_criteria_fields.begin(); it != m_criteria_fields.end(); ++it) {
        auto it2 = snapshot->m_fields.find(it->first);
        if(it2 != snapshot->m_fields.end() && !it2->second.empty()) {
            mismatched_fields[it2->first] = it2->second;
        }
    }

    if(m_resp_msgtype == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP) {
        resp->add_uint16(mismatched_fields.size());
    }

    for(auto it = mismatched_fields.begin(); it != mismatched_fields.end(); ++it) {
        resp->add_uint16(it->first->get_id());
        resp->add_data(it->second);
    }

    m_dbserver->route_datagram(resp);

    delete snapshot;
    cleanup();
}
