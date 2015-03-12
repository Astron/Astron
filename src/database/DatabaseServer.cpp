#include "DatabaseServer.h"

#include "core/global.h"
#include "core/msgtypes.h"
#include "core/shutdown.h"
#include "config/constraints.h"
#include "DatabaseBackend.h"
#include "DBBackendFactory.h"

using namespace std;

static RoleFactoryItem<DatabaseServer> dbserver_fact("database");

RoleConfigGroup dbserver_config("database");
static ConfigVariable<channel_t> control_channel("control", INVALID_CHANNEL, dbserver_config);
static ConfigVariable<bool> broadcast_updates("broadcast", true, dbserver_config);
static InvalidChannelConstraint control_not_invalid(control_channel);
static ReservedChannelConstraint control_not_reserved(control_channel);
static BooleanValueConstraint broadcast_is_boolean(broadcast_updates);

static ConfigGroup generate_config("generate", dbserver_config);
static ConfigVariable<doid_t> min_id("min", INVALID_DO_ID, generate_config);
static ConfigVariable<doid_t> max_id("max", UINT_MAX, generate_config);
static InvalidDoidConstraint min_not_invalid(min_id);
static InvalidDoidConstraint max_not_invalid(max_id);
static ReservedDoidConstraint min_not_reserved(min_id);
static ReservedDoidConstraint max_not_reserved(max_id);

DatabaseServer::DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
    m_control_channel(control_channel.get_rval(roleconfig)),
    m_min_id(min_id.get_rval(roleconfig)),
    m_max_id(max_id.get_rval(roleconfig)),
    m_broadcast(broadcast_updates.get_rval(roleconfig))
{
    ConfigNode generate = dbserver_config.get_child_node(generate_config, roleconfig);
    ConfigNode backend = dbserver_config.get_child_node(db_backend_config, roleconfig);
    m_db_backend = DBBackendFactory::singleton().instantiate_backend(
                       db_backend_type.get_rval(backend), backend,
                       min_id.get_rval(generate), max_id.get_rval(generate));

    // Initialize DatabaseServer log
    stringstream log_title;
    log_title << "Database(" << m_control_channel << ")";
    m_log = new LogCategory("db", log_title.str());
    set_con_name(log_title.str());

    // Check to see the backend was instantiated
    if(!m_db_backend) {
        m_log->fatal() << "No database backend of type '"
                       << db_backend_type.get_rval(backend) << "' exists." << endl;
        astron_shutdown(1);
    }

    // Listen on control channel
    subscribe_channel(m_control_channel);
    subscribe_channel(BCHAN_DBSERVERS);
}

void DatabaseServer::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
    channel_t sender = dgi.read_channel();
    uint16_t msg_type = dgi.read_uint16();

    DBOperation *op;

    switch(msg_type) {
    case DBSERVER_CREATE_OBJECT: {
        op = new DBOperationCreate(this);
    }
    break;
    case DBSERVER_OBJECT_DELETE: {
        op = new DBOperationDelete(this);
    }
    break;
    case DBSERVER_OBJECT_GET_ALL:
    case DBSERVER_OBJECT_GET_FIELD:
    case DBSERVER_OBJECT_GET_FIELDS: {
        op = new DBOperationGet(this);
    }
    break;
    case DBSERVER_OBJECT_SET_FIELD:
    case DBSERVER_OBJECT_SET_FIELDS:
    case DBSERVER_OBJECT_DELETE_FIELD:
    case DBSERVER_OBJECT_DELETE_FIELDS: {
        op = new DBOperationSet(this);
    }
    break;
    case DBSERVER_OBJECT_SET_FIELD_IF_EMPTY:
    case DBSERVER_OBJECT_SET_FIELD_IF_EQUALS:
    case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS: {
        op = new DBOperationUpdate(this);
    }
    break;
    default:
        m_log->error() << "Recieved unknown MsgType: " << msg_type << endl;
        return;
    };

    if(op->initialize(sender, msg_type, dgi)) {
        handle_operation(op);
    }
}

void DatabaseServer::handle_operation(DBOperation *op)
{
    if(op->type() == DBOperation::OperationType::CREATE_OBJECT) {
        // CREATEs do not operate on a specific doId and are therefore non-queued.
        m_db_backend->submit(op);
        return;
    }

    unique_lock<recursive_mutex> guard(m_lock);

    DBOperationQueue &queue = m_queues[op->doid()];

    if(!queue.enqueue_operation(op)) {
        queue.begin_operation(op);
        m_db_backend->submit(op);
    }
}

void DatabaseServer::clear_operation(const DBOperation *op)
{
    if(op->type() == DBOperation::OperationType::CREATE_OBJECT) {
        // CREATEs do not operate on a specific doId and are therefore non-queued.
        return;
    }

    unique_lock<recursive_mutex> guard(m_lock);

    DBOperationQueue &queue = m_queues[op->doid()];

    if(queue.finalize_operation(op)) {
        // The queue says there's a chance this would allow later operations to
        // begin; let's submit all of the eligible operations.
        while(DBOperation *next_op = queue.get_next_operation()) {
            queue.begin_operation(next_op);
            m_db_backend->submit(next_op);
        }
    }

    if(queue.is_empty()) {
        m_queues.erase(op->doid());
    }
}
