#include "OldDatabaseBackend.h"
#include "core/global.h"

void OldDatabaseBackend::submit(DBOperation *operation)
{
    std::lock_guard<std::mutex> lock(m_submit_lock);
    switch(operation->type()) {
    case DBOperation::OperationType::CREATE_OBJECT: {
        ObjectData dbo(operation->dclass()->get_id());
        dbo.fields = operation->set_fields();

        doid_t doid = create_object(dbo);
        if(doid == INVALID_DO_ID || doid < m_min_id || doid > m_max_id) {
            operation->on_failure();
        } else {
            operation->on_complete(doid);
        }
        return;
    }
    break;
    case DBOperation::OperationType::DELETE_OBJECT: {
        delete_object(operation->doid());
        operation->on_complete();
        return;
    }
    break;
    case DBOperation::OperationType::GET_OBJECT:
    case DBOperation::OperationType::GET_FIELDS: {
        ObjectData dbo;
        if(!get_object(operation->doid(), dbo)) {
            operation->on_failure();
            return;
        }

        const dclass::Class *dclass = g_dcf->get_class_by_id(dbo.dc_id);
        if(!dclass || !operation->verify_class(dclass)) {
            operation->on_failure();
            return;
        }

        // Send object to server
        DBObjectSnapshot *snap = new DBObjectSnapshot();
        snap->m_dclass = dclass;
        snap->m_fields = dbo.fields;
        operation->on_complete(snap);
        return;
    }
    break;
    case DBOperation::OperationType::SET_FIELDS: {
        // Check the object's class and fields
        const dclass::Class *dclass = get_class(operation->doid());
        if(!dclass || !operation->verify_class(dclass)) {
            operation->on_failure();
            return;
        }

        // If everthing checks out, update our fields
        for(auto it = operation->set_fields().begin(); it != operation->set_fields().end(); ++it) {
            if(!it->second.empty()) {
                set_field(operation->doid(), it->first, it->second);
            } else {
                del_field(operation->doid(), it->first);
            }
        }

        operation->on_complete();
        return;
    }
    break;
    case DBOperation::UPDATE_FIELDS: {
        // TODO: This can be implemented *much* more efficiently, but for now:
        ObjectData dbo;
        if(!get_object(operation->doid(), dbo)) {
            operation->on_failure();
            return;
        }

        // Make sure all the fields are valid for the class
        const dclass::Class *dclass = g_dcf->get_class_by_id(dbo.dc_id);
        if(!dclass || !operation->verify_class(dclass)) {
            operation->on_failure();
            return;
        }

        // Check to make sure the update is valid
        for(auto it = operation->criteria_fields().begin(); it != operation->criteria_fields().end();
            ++it) {
            if(dbo.fields[it->first] != it->second) {
                DBObjectSnapshot *snap = new DBObjectSnapshot();
                snap->m_fields = dbo.fields;
                operation->on_criteria_mismatch(snap);
                return;
            }
        }

        // Everything checks out, so update the fields
        for(auto it = operation->set_fields().begin(); it != operation->set_fields().end(); ++it) {
            if(!it->second.empty()) {
                set_field(operation->doid(), it->first, it->second);
            } else {
                del_field(operation->doid(), it->first);
            }
        }

        operation->on_complete();
        return;
    }
    }
}
