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

		bool populate_set_fields(DatagramIterator &dgi, uint16_t field_count, bool deletes=false, bool values=false)
		{
			for(uint16_t i = 0; i < field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				const Field *field = g_dcf->get_field_by_id(field_id);
				if(!field)
				{
					m_dbserver->m_log->error() << "Create/modify field request included invalid field #" << field_id << std::endl;
					return false;
				}

				if(field->has_keyword("db"))
				{
					if(values)
					{
						dgi.unpack_field(field, m_criteria_fields[field]);
					}
					if(!deletes)
					{
						dgi.unpack_field(field, m_set_fields[field]);
					}
					else if(field->has_default_value())
					{
						std::string val = field->get_default_value();
						m_set_fields[field] = vector<uint8_t>(val.begin(), val.end());
					}
					else
					{
						m_set_fields[field]; // Force insertion of blank vector
					}
				}
				else
				{
					m_dbserver->m_log->warning() << "Create/modify field request included non-DB field " << field->get_name() << std::endl;
					if(!deletes)
					{
						dgi.skip_field(field);
					}
					if(values)
					{
						// It is not proper to expect a non-db field in criteria.
						return false;
					}
				}
			}

			return true;
		}

		bool populate_get_fields(DatagramIterator &dgi, uint16_t field_count)
		{
			for(uint16_t i = 0; i < field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				const Field *field = g_dcf->get_field_by_id(field_id);
				if(!field)
				{
					m_dbserver->m_log->error() << "Get field request included invalid field #" << field_id << std::endl;
					return false;
				}
				if(field->has_keyword("db"))
				{
					m_get_fields.insert(field);
				}
				else
				{
					m_dbserver->m_log->error() << "Get field request included non-DB field " << field->get_name() << std::endl;
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
				m_dbserver->m_log->error() << "Create object request for invalid dclass ID #" << dclass_id << std::endl;
				on_failure();
				return false;
			}

			uint16_t field_count = dgi.read_uint16();
			if(!populate_set_fields(dgi, field_count))
			{
				on_failure();
				return false;
			}

			// Set all non-present fields to defaults (if they exist)
			for(unsigned int i = 0; i < m_dclass->get_num_fields(); ++i)
			{
				const dclass::Field *field = m_dclass->get_field(i);
				if(field->has_default_value() && field->has_keyword("db") && m_set_fields.find(field) == m_set_fields.end())
				{
					string val = field->get_default_value();
					m_set_fields[field] = vector<uint8_t>(val.begin(), val.end());
				}
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
			// Broadcast update to object's channel
			if(m_dbserver->m_broadcast)
			{
				DatagramPtr update = Datagram::create();
				update->add_server_header(database_to_object(m_doid), m_sender, DBSERVER_OBJECT_DELETE);
				update->add_doid(m_doid);
				m_dbserver->route_datagram(update);
			}
			delete this;
		}
};

class DBOperationImpl_Get : public DBOperationImpl
{
	public:
		DBOperationImpl_Get(DatabaseServer *db) : DBOperationImpl(db) { }

		virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
		{
			m_sender = sender;

			m_context = dgi.read_uint32();
			m_doid = dgi.read_doid();

			if(msg_type == DBSERVER_OBJECT_GET_ALL)
			{
				m_type = GET_OBJECT;
				m_resp_msgtype = DBSERVER_OBJECT_GET_ALL_RESP;
				return true;
			}

			m_type = GET_FIELDS;
			uint16_t field_count;

			if(msg_type == DBSERVER_OBJECT_GET_FIELD)
			{
				field_count = 1;
				m_resp_msgtype = DBSERVER_OBJECT_GET_FIELD_RESP;
			}
			else if(msg_type == DBSERVER_OBJECT_GET_FIELDS)
			{
				field_count = dgi.read_uint16();
				m_resp_msgtype = DBSERVER_OBJECT_GET_FIELDS_RESP;
			}

			return populate_get_fields(dgi, field_count);
		}

		virtual void on_failure()
		{
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(m_sender, m_dbserver->m_control_channel,
			                        m_resp_msgtype);
			resp->add_uint32(m_context);
			resp->add_uint8(FAILURE);
			m_dbserver->route_datagram(resp);

			delete this;
		}

		virtual void on_complete(DBObjectSnapshot *snapshot)
		{
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(m_sender, m_dbserver->m_control_channel,
			                        m_resp_msgtype);
			resp->add_uint32(m_context);
			resp->add_uint8(SUCCESS);

			// Calculate the fields that we are sending in our response:
			std::unordered_map<const dclass::Field*, std::vector<uint8_t> > response_fields;
			if(m_resp_msgtype == DBSERVER_OBJECT_GET_ALL_RESP)
			{
				// Send everything:
				response_fields = snapshot->m_fields;
			}
			else
			{
				// Send only what was requested:
				for(auto it = m_get_fields.begin(); it != m_get_fields.end(); ++it)
				{
					auto it2 = snapshot->m_fields.find(*it);
					if(it2 != snapshot->m_fields.end())
					{
						response_fields[it2->first] = it2->second;
					}
				}
			}

			// WHAT we send depends on our m_resp_msgtype, so:
			if(m_resp_msgtype == DBSERVER_OBJECT_GET_FIELD_RESP)
			{
				if(response_fields.empty())
				{
					// We did not find the field we were looking for.
					// Therefore, this is a failure.
					on_failure();
					return;
				}

				resp->add_uint16 (response_fields.begin()->first->get_id());
				resp->add_data(response_fields.begin()->second);

				// And that's it. We're done.
				m_dbserver->route_datagram(resp);
				delete snapshot;
				delete this;
				return;
			}

			if(m_resp_msgtype == DBSERVER_OBJECT_GET_ALL_RESP)
			{
				resp->add_uint16(snapshot->m_dclass->get_id());
			}

			resp->add_uint16(response_fields.size());
			for(auto it = response_fields.begin(); it != response_fields.end(); ++it)
			{
				resp->add_uint16(it->first->get_id());
				resp->add_data(it->second);
			}

			m_dbserver->route_datagram(resp);
			delete snapshot;
			delete this;
		}

		uint32_t m_context;
		uint16_t m_resp_msgtype;
};

class DBOperationImpl_Modify : public DBOperationImpl
{
	public:
		DBOperationImpl_Modify(DatabaseServer *db) : DBOperationImpl(db) { }

		virtual bool initialize(channel_t sender, uint16_t msg_type, DatagramIterator &dgi)
		{
			m_sender = sender;

			m_type = MODIFY_FIELDS;

			if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EQUALS ||
			   msg_type == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS ||
			   msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EMPTY)
			{
				m_context = dgi.read_uint32();
			}

			m_doid = dgi.read_doid();

			uint16_t field_count;
			if(msg_type == DBSERVER_OBJECT_SET_FIELDS ||
			   msg_type == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS ||
			   msg_type == DBSERVER_OBJECT_DELETE_FIELDS)
			{
				field_count = dgi.read_uint16();
			}
			else
			{
				field_count = 1;
			}

			bool deletes;
			bool values;
			if(msg_type == DBSERVER_OBJECT_SET_FIELD ||
			   msg_type == DBSERVER_OBJECT_SET_FIELDS)
			{
				m_resp_msgtype = 0;
				deletes = false;
				values = false;
			}
			else if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EQUALS ||
			        msg_type == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS)
			{
				m_resp_msgtype = (msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EQUALS) ?
				                 DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP :
				                 DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP;
				deletes = false;
				values = true;
			}
			else if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EMPTY)
			{
				m_resp_msgtype = DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP;
				deletes = false;
				values = false;
			}
			else if(msg_type == DBSERVER_OBJECT_DELETE_FIELD ||
			        msg_type == DBSERVER_OBJECT_DELETE_FIELDS)
			{
				m_resp_msgtype = 0;
				deletes = true;
				values = false;
			}

			if(!populate_set_fields(dgi, field_count, deletes, values))
			{
				on_failure();
				return false;
			}

			if(msg_type == DBSERVER_OBJECT_SET_FIELD_IF_EMPTY)
			{
				// We must satisfy the IF_EMPTY constraint here, because we don't
				// handle it in populate_set_fields.
				m_criteria_fields[m_set_fields.begin()->first]; // Force insertion of blank vector
			}

			return true;
		}

		virtual void on_failure()
		{
			if(!m_resp_msgtype)
			{
				// The request msgtype doesn't have a response.
				delete this;
				return;
			}

			DatagramPtr resp = Datagram::create();
			resp->add_server_header(m_sender, m_dbserver->m_control_channel,
			                        m_resp_msgtype);
			resp->add_uint32(m_context);
			resp->add_uint8(FAILURE);
			m_dbserver->route_datagram(resp);

			delete this;
		}

		virtual void on_criteria_mismatch(DBObjectSnapshot *snapshot)
		{
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(m_sender, m_dbserver->m_control_channel,
			                        m_resp_msgtype);
			resp->add_uint32(m_context);
			resp->add_uint8(FAILURE);

			// Calculate the fields that we are sending in our response:
			std::unordered_map<const dclass::Field*, std::vector<uint8_t> > mismatched_fields;

			for(auto it = m_criteria_fields.begin(); it != m_criteria_fields.end(); ++it)
			{
				auto it2 = snapshot->m_fields.find(it->first);
				if(it2 != snapshot->m_fields.end() && !it2->second.empty())
				{
					mismatched_fields[it2->first] = it2->second;
				}
			}

			if(m_resp_msgtype == DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP)
			{
				resp->add_uint16(mismatched_fields.size());
			}

			for(auto it = mismatched_fields.begin(); it != mismatched_fields.end(); ++it)
			{
				resp->add_uint16(it->first->get_id());
				resp->add_data(it->second);
			}

			m_dbserver->route_datagram(resp);

			delete snapshot;
			delete this;
		}

		virtual void on_complete()
		{
			// Broadcast update to object's channel
			if(m_dbserver->m_broadcast)
			{
				// Calculate the fields that we are sending in our response:
				std::unordered_map<const dclass::Field*, std::vector<uint8_t> > changed_fields;
				std::unordered_set<const dclass::Field*> deleted_fields;

				for(auto it = m_set_fields.begin(); it != m_set_fields.end(); ++it)
				{
					if(it->second.empty())
					{
						deleted_fields.insert(it->first);
					}
					else
					{
						changed_fields[it->first] = it->second;
					}
				}

				if(!deleted_fields.empty())
				{
					bool multi = (deleted_fields.size() > 1);
					DatagramPtr update = Datagram::create();
					update->add_server_header(database_to_object(m_doid), m_sender,
					                          multi ? DBSERVER_OBJECT_DELETE_FIELDS :
					                                  DBSERVER_OBJECT_DELETE_FIELD);
					update->add_doid(m_doid);
					if(multi)
					{
						update->add_uint16(deleted_fields.size());
					}
					for(auto it = deleted_fields.begin(); it != deleted_fields.end(); ++it)
					{
						update->add_uint16((*it)->get_id());
					}
					m_dbserver->route_datagram(update);
				}

				if(!changed_fields.empty())
				{
					bool multi = (changed_fields.size() > 1);
					DatagramPtr update = Datagram::create();
					update->add_server_header(database_to_object(m_doid), m_sender,
					                          multi ? DBSERVER_OBJECT_SET_FIELDS :
					                                  DBSERVER_OBJECT_SET_FIELD);
					update->add_doid(m_doid);
					if(multi)
					{
						update->add_uint16(changed_fields.size());
					}
					for(auto it = changed_fields.begin(); it != changed_fields.end(); ++it)
					{
						update->add_uint16(it->first->get_id());
						update->add_data(it->second);
					}
					m_dbserver->route_datagram(update);
				}
			}

			if(m_resp_msgtype)
			{
				DatagramPtr resp = Datagram::create();
				resp->add_server_header(m_sender, m_dbserver->m_control_channel,
				                        m_resp_msgtype);
				resp->add_uint32(m_context);
				resp->add_uint8(SUCCESS);
				m_dbserver->route_datagram(resp);
			}

			delete this;
		}

		uint32_t m_context;
		uint16_t m_resp_msgtype;
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

void DatabaseServer::handle_operation(DBOperation *op)
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
			op = new DBOperationImpl_Get(this);
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
			op = new DBOperationImpl_Modify(this);
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
