#include "DatabaseServer.h"

#include "core/global.h"
#include "core/msgtypes.h"
#include "config/constraints.h"
#include "DBBackendFactory.h"
#include "DatabaseBackend.h"

using namespace std;
using dclass::Field;
using dclass::Class;

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

class DBOperationImpl : public DBOperation
{
	public:
		DBOperationImpl(DatabaseServer *db) : m_dbserver(db) { }

		virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi) = 0;

		bool populate_set_fields(DatagramIterator &dgi)
		{
			uint16_t field_count = dgi.read_uint16();
			for(uint16_t i = 0; i < field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				const Field *field = g_dcf->get_field_by_id(field_id);
				if(!field)
				{
					// TODO: Error!
					return false;
				}
				if(field->has_keyword("db"))
				{
					dgi.unpack_field(field, m_set_fields[field]);
				}
				else
				{
					// TODO: Warning!
					dgi.skip_field(field);
				}
			}

			return true;
		}

		DatabaseServer *m_dbserver;
		channel_t m_sender;
};

class DBOperationImpl_Create : public DBOperationImpl
{
	public:
		DBOperationImpl_Create(DatabaseServer *db) : DBOperationImpl(db) { }

		virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
		{
			m_sender = sender;

			m_type = CREATE_OBJECT;
			m_context = dgi.read_uint32();

			uint16_t dclass_id = dgi.read_uint16();
			m_dclass = g_dcf->get_class_by_id(dclass_id);
			if(!m_dclass)
			{
				// TODO: Error!
				on_failure();
				return false;
			}

			if(!populate_set_fields(dgi))
			{
				on_failure();
				return false;
			}

			return true;
		}

		virtual void on_complete(doid_t doid)
		{
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(m_sender, m_dbserver->m_control_channel,
									DBSERVER_CREATE_OBJECT_RESP);
			resp->add_uint32(m_context);
			resp->add_doid(doid);
			m_dbserver->route_datagram(resp);

			delete this;
		}

		virtual void on_failure()
		{
			// We just send back an invalid response...
			on_complete(INVALID_DO_ID);
		}

		uint32_t m_context;
};

class DBOperationImpl_Delete : public DBOperationImpl
{
	public:
		DBOperationImpl_Delete(DatabaseServer *db) : DBOperationImpl(db) { }

		virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
		{
			m_sender = sender;

			m_type = DELETE_OBJECT;
			m_doid = dgi.read_doid();

			return true;
		}

		virtual void on_failure()
		{
			delete this;
		}

		virtual void on_complete()
		{
			delete this;
		}
};

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
	if(!m_db_backend)
	{
		m_log->fatal() << "No database backend of type '"
		               << db_backend_type.get_rval(backend) << "' exists." << endl;
		exit(1);
	}

	// Listen on control channel
	subscribe_channel(m_control_channel);
}

void DatabaseServer::handle_operation(DBOperationImpl *op)
{
	m_db_backend->submit(op);
}

void DatabaseServer::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_channel();
	uint16_t msg_type = dgi.read_uint16();

	DBOperationImpl *op;

	switch(msg_type)
	{
		case DBSERVER_CREATE_OBJECT:
		{
			op = new DBOperationImpl_Create(this);
		}
		break;
		case DBSERVER_OBJECT_DELETE:
		{
			op = new DBOperationImpl_Delete(this);
		}
		break;
		case DBSERVER_OBJECT_GET_ALL:
		case DBSERVER_OBJECT_GET_FIELD:
		case DBSERVER_OBJECT_GET_FIELDS:
		{
			//op = new DBOperationImpl_Get(this);
		}
		break;
		case DBSERVER_OBJECT_SET_FIELD:
		case DBSERVER_OBJECT_SET_FIELDS:
		case DBSERVER_OBJECT_SET_FIELD_IF_EMPTY:
		case DBSERVER_OBJECT_SET_FIELD_IF_EQUALS:
		case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS:
		case DBSERVER_OBJECT_DELETE_FIELD:
		case DBSERVER_OBJECT_DELETE_FIELDS:
		{
			//op = new DBOperationImpl_Modify(this);
		}
		break;
		default:
			m_log->error() << "Recieved unknown MsgType: " << msg_type << endl;
			return;
	};

	if(op->initialize(sender, msg_type, dgi))
	{
		handle_operation(op);
	}
}
