#include "core/global.h"
#include "core/msgtypes.h"
#include "config/constraints.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"
#include <unordered_set>

#include "DBStateServer.h"
#include "LoadingObject.h"

using dclass::Class;
using dclass::Field;

static RoleFactoryItem<DBStateServer> dbss_fact("dbss");

static RoleConfigGroup dbss_config("dbss");
static ConfigVariable<channel_t> database_channel("database", INVALID_CHANNEL, dbss_config);
static InvalidChannelConstraint db_channel_not_invalid(database_channel);
static ReservedChannelConstraint db_channel_not_reserved(database_channel);

static ConfigList ranges_config("ranges", dbss_config);
static ConfigVariable<doid_t> range_min("min", INVALID_DO_ID, ranges_config);
static ConfigVariable<doid_t> range_max("max", DOID_MAX, ranges_config);
static InvalidDoidConstraint min_not_invalid(range_min);
static InvalidDoidConstraint max_not_invalid(range_max);
static ReservedDoidConstraint min_not_reserved(range_min);
static ReservedDoidConstraint max_not_reserved(range_max);

DBStateServer::DBStateServer(RoleConfig roleconfig) : StateServer(roleconfig),
    m_db_channel(database_channel.get_rval(m_roleconfig)), m_next_context(0)
{
    ConfigNode ranges = dbss_config.get_child_node(ranges_config, roleconfig);
    for(const auto& it : ranges) {
        channel_t min = range_min.get_rval(it);
        channel_t max = range_max.get_rval(it);
        subscribe_range(min, max);
    }

    std::stringstream name;
    name << "DBSS(Database: " << m_db_channel << ")";
    m_log = std::unique_ptr<LogCategory>(new LogCategory("dbss", name.str()));
    set_con_name(name.str());
}

void DBStateServer::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
    channel_t sender = dgi.read_channel();
    uint16_t msgtype = dgi.read_uint16();
    switch(msgtype) {
    case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS:
        handle_activate(dgi, false);
        break;
    case DBSS_OBJECT_ACTIVATE_WITH_DEFAULTS_OTHER:
        handle_activate(dgi, true);
        break;
    case DBSS_OBJECT_DELETE_DISK:
        handle_delete_disk(sender, dgi);
        break;
    case STATESERVER_OBJECT_SET_FIELD:
        handle_set_field(dgi);
        break;
    case STATESERVER_OBJECT_SET_FIELDS:
        handle_set_fields(dgi);
        break;
    case STATESERVER_OBJECT_GET_FIELD:
        handle_get_field(sender, dgi);
        break;
    case DBSERVER_OBJECT_GET_FIELD_RESP:
        handle_get_field_resp(dgi);
        break;
    case STATESERVER_OBJECT_GET_FIELDS:
        handle_get_fields(sender, dgi);
        break;
    case DBSERVER_OBJECT_GET_FIELDS_RESP:
        handle_get_fields_resp(dgi);
        break;
    case STATESERVER_OBJECT_GET_ALL:
        handle_get_all(sender, dgi);
        break;
    case DBSERVER_OBJECT_GET_ALL_RESP:
        handle_get_all_resp(dgi);
        break;
    case DBSS_OBJECT_GET_ACTIVATED:
        handle_get_activated(sender, dgi);
        break;
    default:
        m_log->trace() << "Ignoring message of type '" << msgtype << "'.\n";
    }
}

void DBStateServer::handle_activate(DatagramIterator &dgi, bool has_other)
{
    doid_t do_id = dgi.read_doid();
    doid_t parent_id = dgi.read_doid();
    zone_t zone_id = dgi.read_zone();

    // Check object is not already active
    if(m_objs.find(do_id) != m_objs.end() || m_loading.find(do_id) != m_loading.end()) {
        m_log->warning() << "Received activate for already-active object with id " << do_id << "\n";
        return;
    }

    if(!has_other) {
        auto load_it = m_inactive_loads.find(do_id);
        if(load_it == m_inactive_loads.end()) {
            m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id);
            m_loading[do_id]->begin();
        } else {
            m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id, load_it->second);
        }
    } else {
        uint16_t dc_id = dgi.read_uint16();

        // Check id is a valid type id
        if(dc_id >= g_dcf->get_num_types()) {
            m_log->error() << "Received activate_other with unknown dclass"
                           " with id " << dc_id << "\n";
            return;
        }

        const Class *dcc = g_dcf->get_class_by_id(dc_id);
        if(!dcc) {
            m_log->error() << "Tried to activate_other with non-class distributed_type #" << dc_id << "\n";
        }
        auto load_it = m_inactive_loads.find(do_id);
        if(load_it == m_inactive_loads.end()) {
            m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id, dcc, dgi);
            m_loading[do_id]->begin();
        } else {
            m_loading[do_id] = new LoadingObject(this, do_id, parent_id, zone_id, dcc, dgi, load_it->second);
        }
    }
}

void DBStateServer::handle_get_activated(channel_t sender, DatagramIterator& dgi)
{
    uint32_t r_context = dgi.read_uint32();
    doid_t r_do_id = dgi.read_doid();
    if(m_loading.find(r_do_id) != m_loading.end()) {
        return;
    }

    m_log->trace() << "Received GetActivated for id " << r_do_id << "\n";

    if(m_objs.find(r_do_id) != m_objs.end()) {
        // If object is active return true
        DatagramPtr dg = Datagram::create(sender, r_do_id, DBSS_OBJECT_GET_ACTIVATED_RESP);
        dg->add_uint32(r_context);
        dg->add_doid(r_do_id);
        dg->add_bool(true);
        route_datagram(dg);
    } else {
        // If object isn't active or loading, we can return false
        DatagramPtr dg = Datagram::create(sender, r_do_id, DBSS_OBJECT_GET_ACTIVATED_RESP);
        dg->add_uint32(r_context);
        dg->add_doid(r_do_id);
        dg->add_bool(false);
        route_datagram(dg);
    }
}

void DBStateServer::handle_delete_disk(channel_t sender, DatagramIterator& dgi)
{
    doid_t do_id = dgi.read_doid();
    if(m_loading.find(do_id) != m_loading.end()) {
        // Ignore this message for now, it'll be bounced back to us
        // from the loading object if it succeeds or fails at loading.
        return;
    }

    // If object exists broadcast the delete message
    auto obj_keyval = m_objs.find(do_id);
    if(obj_keyval != m_objs.end()) {
        DistributedObject* obj = obj_keyval->second;
        std::set<channel_t> targets;

        // Add location to broadcast
        if(obj->get_location()) {
            targets.insert(obj->get_location());
        }

        // Add AI to broadcast
        if(obj->get_ai()) {
            targets.insert(obj->get_ai());
        }

        // Add owner to broadcast
        if(obj->get_owner()) {
            targets.insert(obj->get_owner());
        }

        // Build and send datagram
        DatagramPtr dg = Datagram::create(targets, sender, DBSS_OBJECT_DELETE_DISK);
        dg->add_doid(do_id);
        route_datagram(dg);
    }

    // Send delete to database
    DatagramPtr dg = Datagram::create(m_db_channel, do_id, DBSERVER_OBJECT_DELETE);
    dg->add_doid(do_id);
    route_datagram(dg);

}

void DBStateServer::handle_set_field(DatagramIterator &dgi)
{
    doid_t do_id = dgi.read_doid();
    if(m_loading.find(do_id) != m_loading.end()) {
        // Ignore this message for now, it'll be bounced back to us
        // from the loading object if it succeeds or fails at loading.
        return;
    }

    uint16_t field_id = dgi.read_uint16();

    const Field* field = g_dcf->get_field_by_id(field_id);
    if(field && field->has_keyword("db")) {
        m_log->trace() << "Forwarding SetField for field \"" << field->get_name()
                       << "\" on object with id " << do_id << " to database.\n";

        DatagramPtr dg = Datagram::create(m_db_channel, do_id, DBSERVER_OBJECT_SET_FIELD);
        dg->add_doid(do_id);
        dg->add_uint16(field_id);
        dg->add_data(dgi.read_remainder());
        route_datagram(dg);
    }
}

void DBStateServer::handle_set_fields(DatagramIterator &dgi)
{
    doid_t do_id = dgi.read_doid();
    if(m_loading.find(do_id) != m_loading.end()) {
        // Ignore this message for now, it'll be bounced back to us
        // from the loading object if it succeeds or fails at loading.
        return;
    }

    uint16_t field_count = dgi.read_uint16();

    FieldValues db_fields;
    for(uint16_t i = 0; i < field_count; ++i) {
        uint16_t field_id = dgi.read_uint16();
        const Field* field = g_dcf->get_field_by_id(field_id);
        if(!field) {
            m_log->warning() << "Received invalid field with id " << field_id << " in SetFields.\n";
            return;
        }
        if(field->has_keyword("db")) {
            dgi.unpack_field(field, db_fields[field]);
        } else {
            dgi.skip_field(field);
        }
    }

    if(db_fields.size() > 0) {
        m_log->trace() << "Forwarding SetFields on object with id " << do_id << " to database.\n";

        DatagramPtr dg = Datagram::create(m_db_channel, do_id, DBSERVER_OBJECT_SET_FIELDS);
        dg->add_doid(do_id);
        dg->add_uint16(db_fields.size());
        for(const auto& it : db_fields) {
            dg->add_uint16(it.first->get_id());
            dg->add_data(it.second);
        }
        route_datagram(dg);
    }
}

void DBStateServer::handle_get_field(channel_t sender, DatagramIterator &dgi)
{
    uint32_t r_context = dgi.read_uint32();
    doid_t r_do_id = dgi.read_doid();
    uint16_t field_id = dgi.read_uint16();
    if(is_activated_object(r_do_id)) {
        return;
    }

    m_log->trace() << "Received GetField for field with id " << field_id
                   << " on inactive object with id " << r_do_id << "\n";

    // Check field is "ram db" or "required"
    const Field* field = g_dcf->get_field_by_id(field_id);
    if(!field || !(field->has_keyword("required") || field->has_keyword("ram"))) {
        DatagramPtr dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
        dg->add_uint32(r_context);
        dg->add_bool(false);
        route_datagram(dg);
        return;
    }

    if(field->has_keyword("db")) {
        // Get context for db query
        uint32_t db_context = m_next_context++;

        // Prepare reponse datagram
        DatagramPtr dg_resp = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
        dg_resp->add_uint32(r_context);
        m_context_datagrams[db_context] = dg_resp;

        // Send query to database
        DatagramPtr dg = Datagram::create(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_FIELD);
        dg->add_uint32(db_context);
        dg->add_doid(r_do_id);
        dg->add_uint16(field_id);
        route_datagram(dg);
    } else if(field->has_default_value()) { // Field is required and not-db
        DatagramPtr dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
        dg->add_uint32(r_context);
        dg->add_bool(true);
        dg->add_uint16(field_id);
        dg->add_data(field->get_default_value());
        route_datagram(dg);
    } else {
        DatagramPtr dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELD_RESP);
        dg->add_uint32(r_context);
        dg->add_bool(false);
        route_datagram(dg);
    }
}

void DBStateServer::handle_get_field_resp(DatagramIterator& dgi)
{
    uint32_t db_context = dgi.read_uint32();
    if(!is_expected_context(db_context)) {
        return;
    }

    // Get the datagram from the db_context
    DatagramPtr dg = m_context_datagrams[db_context];
    m_context_datagrams.erase(db_context);

    // Check to make sure the datagram is appropriate
    DatagramIterator check_dgi = DatagramIterator(dg);
    uint16_t resp_type = check_dgi.get_msg_type();
    if(resp_type != STATESERVER_OBJECT_GET_FIELD_RESP) {
        if(resp_type == STATESERVER_OBJECT_GET_FIELDS_RESP) {
            m_log->warning() << "Received GetFieldsResp, but expecting GetFieldResp." << std::endl;
        } else if(resp_type == STATESERVER_OBJECT_GET_ALL_RESP) {
            m_log->warning() << "Received GetAllResp, but expecting GetFieldResp." << std::endl;
        }
        return;
    }

    m_log->trace() << "Received GetFieldResp from database." << std::endl;

    // Add database field payload to response (don't know dclass, so must copy payload) and send
    dg->add_data(dgi.read_remainder());
    route_datagram(dg);
}

void DBStateServer::handle_get_fields(channel_t sender, DatagramIterator &dgi)
{
    uint32_t r_context = dgi.read_uint32();
    doid_t r_do_id = dgi.read_doid();
    uint16_t field_count = dgi.read_uint16();
    if(is_activated_object(r_do_id)) {
        return;
    }

    m_log->trace() << "Received GetFields for inactive object with id " << r_do_id << std::endl;

    // Read requested fields from datagram
    std::list<const Field*> db_fields; // Ram|required db fields in request
    std::list<const Field*> ram_fields; // Ram|required but not-db fields in request
    for(uint16_t i = 0; i < field_count; ++i) {
        uint16_t field_id = dgi.read_uint16();
        const Field* field = g_dcf->get_field_by_id(field_id);
        if(!field) {
            DatagramPtr dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELDS_RESP);
            dg->add_uint32(r_context);
            dg->add_uint8(false);
            route_datagram(dg);
        } else if(field->has_keyword("ram") || field->has_keyword("required")) {
            if(field->has_keyword("db")) {
                db_fields.push_back(field);
            } else {
                ram_fields.push_back(field);
            }
        }
    }

    if(db_fields.size()) {
        // Get context for db query
        uint32_t db_context = m_next_context++;

        // Prepare reponse datagram
        if(m_context_datagrams.find(db_context) == m_context_datagrams.end()) {
            m_context_datagrams[db_context] = Datagram::create(sender, r_do_id,
                                              STATESERVER_OBJECT_GET_FIELDS_RESP);
        }
        m_context_datagrams[db_context]->add_uint32(r_context);
        m_context_datagrams[db_context]->add_bool(true);
        m_context_datagrams[db_context]->add_uint16(ram_fields.size() + db_fields.size());
        for(const auto& it : ram_fields) {
            m_context_datagrams[db_context]->add_uint16(it->get_id());
            m_context_datagrams[db_context]->add_data(it->get_default_value());
        }

        // Send query to database
        DatagramPtr dg = Datagram::create(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_FIELDS);
        dg->add_uint32(db_context);
        dg->add_doid(r_do_id);
        dg->add_uint16(db_fields.size());
        for(const auto& it : db_fields) {
            dg->add_uint16(it->get_id());
        }
        route_datagram(dg);
    } else { // If no database fields exist
        DatagramPtr dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_FIELDS_RESP);
        dg->add_uint32(r_context);
        dg->add_bool(true);
        dg->add_uint16(ram_fields.size());
        for(const auto& it : ram_fields) {
            dg->add_uint16(it->get_id());
            dg->add_data(it->get_default_value());
        }
        route_datagram(dg);
    }
}

void DBStateServer::handle_get_fields_resp(DatagramIterator& dgi)
{
    uint32_t db_context = dgi.read_uint32();
    if(!is_expected_context(db_context)) {
        return;
    }

    // Get the datagram from the db_context
    DatagramPtr dg = m_context_datagrams[db_context];
    m_context_datagrams.erase(db_context);

    // Check to make sure the datagram is appropriate
    DatagramIterator check_dgi = DatagramIterator(dg);
    uint16_t resp_type = check_dgi.get_msg_type();
    if(resp_type != STATESERVER_OBJECT_GET_FIELDS_RESP) {
        if(resp_type == STATESERVER_OBJECT_GET_FIELD_RESP) {
            m_log->warning() << "Received GetFieldResp, but expecting GetFieldsResp." << std::endl;
        } else if(resp_type == STATESERVER_OBJECT_GET_ALL_RESP) {
            m_log->warning() << "Received GetAllResp, but expecting GetFieldsResp." << std::endl;
        }
        return;
    }

    m_log->trace() << "Received GetFieldResp from database." << std::endl;

    // Add database field payload to response (don't know dclass, so must copy payload).
    if(dgi.read_bool() == true) {
        dgi.read_uint16(); // Discard field count
        dg->add_data(dgi.read_remainder());
    }
    route_datagram(dg);
}


void DBStateServer::handle_get_all(channel_t sender, DatagramIterator &dgi)
{
    uint32_t r_context = dgi.read_uint32();
    doid_t r_do_id = dgi.read_doid();
    if(is_activated_object(r_do_id)) {
        return;
    }

    m_log->trace() << "Received GetAll for inactive object with id " << r_do_id << std::endl;

    // Get context for db query, and remember caller with it
    uint32_t db_context = m_next_context++;

    DatagramPtr resp_dg = Datagram::create(sender, r_do_id, STATESERVER_OBJECT_GET_ALL_RESP);
    resp_dg->add_uint32(r_context);
    resp_dg->add_doid(r_do_id);
    resp_dg->add_channel(INVALID_CHANNEL); // Location
    m_context_datagrams[db_context] = resp_dg;

    // Cache the do_id --> context in case we get a dbss_activate
    m_inactive_loads[r_do_id].insert(db_context);

    // Send query to database
    DatagramPtr dg = Datagram::create(m_db_channel, r_do_id, DBSERVER_OBJECT_GET_ALL);
    dg->add_uint32(db_context);
    dg->add_doid(r_do_id);
    route_datagram(dg);
}

void DBStateServer::handle_get_all_resp(DatagramIterator& dgi)
{
    uint32_t db_context = dgi.read_uint32();
    if(!is_expected_context(db_context)) {
        return;
    }

    // Get the datagram from the db_context
    DatagramPtr dg = m_context_datagrams[db_context];
    m_context_datagrams.erase(db_context);

    // Check to make sure the datagram is appropriate
    DatagramIterator check_dgi = DatagramIterator(dg);
    uint16_t resp_type = check_dgi.get_msg_type();
    if(resp_type != STATESERVER_OBJECT_GET_ALL_RESP) {
        if(resp_type == STATESERVER_OBJECT_GET_FIELD_RESP) {
            m_log->warning() << "Received GetFieldResp, but expecting GetAllResp." << std::endl;
        } else if(resp_type == STATESERVER_OBJECT_GET_FIELDS_RESP) {
            m_log->warning() << "Received GetFieldsResp, but expecting GetAllResp." << std::endl;
        }
        return;
    }

    // Get do_id from datagram
    check_dgi.seek_payload();
    check_dgi.skip(sizeof(channel_t) + sizeof(doid_t)); // skip over sender and context to do_id;
    doid_t do_id = check_dgi.read_doid();

    // Remove cached loading operation
    if(m_inactive_loads[do_id].size() > 1) {
        m_inactive_loads[do_id].erase(db_context);
    } else {
        m_inactive_loads.erase(do_id);
    }

    m_log->trace() << "Received GetAllResp from database." << std::endl;

    // If object not found, just cleanup the context map
    if(dgi.read_bool() != true) {
        return; // Object not found
    }

    // Read object class
    uint16_t dc_id = dgi.read_uint16();
    if(!dc_id) {
        m_log->error() << "Received object from database with unknown dclass"
                       << " - id:" << dc_id << std::endl;
        return;
    }
    const Class* r_class = g_dcf->get_class_by_id(dc_id);

    // Get fields from database
    UnorderedFieldValues required_fields;
    FieldValues ram_fields;
    if(!unpack_db_fields(dgi, r_class, required_fields, ram_fields)) {
        m_log->error() << "Error while unpacking fields from database." << std::endl;
        return;
    }

    // Add class to response
    dg->add_uint16(r_class->get_id());

    // Add required fields to datagram
    int dcc_field_count = r_class->get_num_fields();
    for(int i = 0; i < dcc_field_count; ++i) {
        const Field *field = r_class->get_field(i);
        if(!field->as_molecular() && field->has_keyword("required")) {
            auto req_it = required_fields.find(field);
            if(req_it != required_fields.end()) {
                dg->add_data(req_it->second);
            } else {
                dg->add_data(field->get_default_value());
            }
        }
    }

    // Add ram fields to datagram
    dg->add_uint16(ram_fields.size());
    for(const auto& it : ram_fields) {
        dg->add_uint16(it.first->get_id());
        dg->add_data(it.second);
    }

    // Send response back to caller
    route_datagram(dg);
}

void DBStateServer::receive_object(DistributedObject* obj)
{
    m_objs[obj->get_id()] = obj;
}

void DBStateServer::discard_loader(doid_t do_id)
{
    m_loading.erase(do_id);
}

bool DBStateServer::is_expected_context(uint32_t context)
{
    return m_context_datagrams.find(context) != m_context_datagrams.end();
}

bool DBStateServer::is_activated_object(doid_t do_id)
{
    return m_objs.find(do_id) != m_objs.end() || m_loading.find(do_id) != m_loading.end();
}


bool unpack_db_fields(DatagramIterator &dgi, const Class* dc_class,
                      UnorderedFieldValues &required, FieldValues &ram)
{
    // Unload ram and required fields from database resp
    uint16_t db_field_count = dgi.read_uint16();
    for(uint16_t i = 0; i < db_field_count; ++i) {
        uint16_t field_id = dgi.read_uint16();
        const Field *field = dc_class->get_field_by_id(field_id);
        if(!field) {
            return false;
        }
        if(field->has_keyword("required")) {
            dgi.unpack_field(field, required[field]);
        } else if(field->has_keyword("ram")) {
            dgi.unpack_field(field, ram[field]);
        } else {
            dgi.skip_field(field);
        }
    }

    return true;
}
