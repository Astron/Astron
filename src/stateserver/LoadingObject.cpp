#include "core/global.h"
#include "core/msgtypes.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"

#include "LoadingObject.h"

using dclass::Class;
using dclass::Field;

LoadingObject::LoadingObject(DBStateServer *stateserver, doid_t do_id,
                             doid_t parent_id, zone_t zone_id,
                             const std::unordered_set<uint32_t> &contexts) :
    m_dbss(stateserver), m_do_id(do_id), m_parent_id(parent_id), m_zone_id(zone_id),
    m_context(stateserver->m_next_context++), m_dclass(nullptr), m_valid_contexts(contexts),
    m_is_loaded(false)
{
    std::stringstream name;
    name << "LoadingObject(doid: " << do_id << ", db: " << m_dbss->m_db_channel << ")";
    m_log = std::unique_ptr<LogCategory>(new LogCategory("dbobject", name.str()));
    set_con_name(name.str());

    subscribe_channel(do_id);
}

LoadingObject::LoadingObject(DBStateServer *stateserver, doid_t do_id, doid_t parent_id,
                             zone_t zone_id, const Class *dclass, DatagramIterator &dgi,
                             const std::unordered_set<uint32_t> &contexts) :
    m_dbss(stateserver), m_do_id(do_id), m_parent_id(parent_id), m_zone_id(zone_id),
    m_context(stateserver->m_next_context++), m_dclass(dclass), m_valid_contexts(contexts),
    m_is_loaded(false)
{
    std::stringstream name;
    name << "LoadingObject(doid: " << do_id << ", db: " << m_dbss->m_db_channel << ")";
    m_log = std::unique_ptr<LogCategory>(new LogCategory("dbobject", name.str()));
    set_con_name(name.str());

    subscribe_channel(do_id);

    uint16_t field_count = dgi.read_uint16();
    for(uint16_t i{}; i < field_count; ++i) {
        uint16_t field_id = dgi.read_uint16();
        const Field *field = m_dclass->get_field_by_id(field_id);

        if(!field) {
            m_log->error() << "Received invalid field index " << field_id << std::endl;
            return;
        }

        if(field->has_keyword("ram") || field->has_keyword("required")) {
            dgi.unpack_field(field, m_field_updates[field]);
        } else {
            m_log->error() << "Received non-RAM field " << field->get_name()
                           << " within an OTHER section.\n";
        }
    }
}

void LoadingObject::begin()
{
    if(!m_valid_contexts.size()) {
        send_get_object(m_do_id);
    }
}

void LoadingObject::send_get_object(doid_t do_id)
{
    DatagramPtr dg = Datagram::create(m_dbss->m_db_channel, do_id, DBSERVER_OBJECT_GET_ALL);
    dg->add_uint32(m_context); // Context
    dg->add_doid(do_id);
    route_datagram(dg);
}

// replay_datagrams emits datagrams from the loading object queue in the order
// they were received when the object is successully loaded
void LoadingObject::replay_datagrams(DistributedObject* obj)
{
    m_log->trace() << "Replaying datagrams received while loading...\n";
    for(const auto& it : m_datagram_queue) {
        if(m_dbss->m_objs.find(m_do_id) == m_dbss->m_objs.end()) {
            m_log->trace() << "... deleted while replaying, aborted.\n";
            return;
        }
        try {
            DatagramIterator dgi(it);
            dgi.seek_payload();
            obj->handle_datagram(it, dgi);
        } catch(const DatagramIteratorEOF&) {
            m_log->error() << "Detected truncated datagram while replaying"
                           " datagrams to object, from loaded object. Skipped.\n";
        }
    }
    m_log->trace() << "... replay finished.\n";
}

// forward_datagrams emits datagrams builtup in the queue to the DBSS if a
// loading object failed to properly load.
void LoadingObject::forward_datagrams()
{
    m_log->trace() << "Forwarding datagrams received while loading...\n";
    for(const auto& it : m_datagram_queue) {
        try {
            DatagramIterator dgi(it);
            dgi.seek_payload();
            m_dbss->handle_datagram(it, dgi);
        } catch(const DatagramIteratorEOF&) {
            m_log->error() << "Detected truncated datagram while replaying"
                           " datagrams to dbss, from failed loading object. Skipped.\n";
        }
    }
    m_log->trace() << "... forwarding finished.\n";
}

void LoadingObject::finalize()
{
    m_dbss->discard_loader(m_do_id);
    forward_datagrams();
    terminate();
}

void LoadingObject::handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi)
{
    /*channel_t sender =*/ dgi.read_channel(); // sender not used
    uint16_t msgtype = dgi.read_uint16();
    switch(msgtype) {
    case DBSERVER_OBJECT_GET_ALL_RESP: {
        if(m_is_loaded) {
            break; // Don't care about these message any more if loaded
        }

        uint32_t db_context = dgi.read_uint32();
        if(db_context != m_context &&
           m_valid_contexts.find(db_context) == m_valid_contexts.end()) {
            m_log->warning() << "Received get_all_resp with incorrect context '"
                             << db_context << "'.\n";
            break;
        }

        m_log->trace() << "Received GetAllResp from database.\n";
        m_is_loaded = true;

        if(dgi.read_bool() != true) {
            m_log->debug() << "Object not found in database.\n";
            finalize();
            break;
        }

        uint16_t dc_id = dgi.read_uint16();
        const Class *r_dclass = g_dcf->get_class_by_id(dc_id);
        if(!r_dclass) {
            m_log->error() << "Received object from database with unknown dclass"
                           << " - id:" << dc_id << std::endl;
            finalize();
            break;
        }

        if(m_dclass && r_dclass != m_dclass) {
            m_log->error() << "Requested object of class '" << m_dclass->get_id()
                           << "', but received class " << dc_id << std::endl;
            finalize();
            break;
        }

        // Get fields from database
        if(!unpack_db_fields(dgi, r_dclass, m_required_fields, m_ram_fields)) {
            m_log->error() << "Error while unpacking fields from database.\n";
            finalize();
            break;
        }

        // Add default values and updated values
        std::size_t dcc_field_count = r_dclass->get_num_fields();
        for(std::size_t i{}; i < dcc_field_count; ++i) {
            const Field *field = r_dclass->get_field(i);
            if(!field->as_molecular()) {
                if(field->has_keyword("required")) {
                    if(m_field_updates.find(field) != m_field_updates.end()) {
                        m_required_fields[field] = m_field_updates[field];
                    } else if(m_required_fields.find(field) == m_required_fields.end()) {
                        std::string val = field->get_default_value();
                        m_required_fields[field] = std::vector<uint8_t>(val.begin(), val.end());
                    }
                } else if(field->has_keyword("ram")) {
                    if(m_field_updates.find(field) != m_field_updates.end()) {
                        m_ram_fields[field] = m_field_updates[field];
                    }
                }
            }
        }

        // Create object on stateserver
        DistributedObject* obj = new DistributedObject(m_dbss, m_dbss->m_db_channel, m_do_id,
                m_parent_id, m_zone_id, r_dclass,
                m_required_fields, m_ram_fields);

        // Tell DBSS about object and handle datagram queue
        m_dbss->receive_object(obj);
        replay_datagrams(obj);

        // Cleanup this loader
        finalize();
        break;
    }
    case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS:
    case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER: {
        // Don't cache these messages in the queue, they are received and
        // handled by the DBSS.  Since the object is already loading they
        // are simply ignored (the DBSS may generate a warning/error).
        break;
    }
    default: {
        m_datagram_queue.push_back(in_dg);
    }
    }
}
