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

void DatabaseServer::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_channel();
	uint16_t msg_type = dgi.read_uint16();
	switch(msg_type)
	{
		case DBSERVER_CREATE_OBJECT:
		{
			uint32_t context = dgi.read_uint32();

			// Start response with generic header
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel, DBSERVER_CREATE_OBJECT_RESP);
			resp->add_uint32(context);

			// Get DistributedClass
			uint16_t dc_id = dgi.read_uint16();
			const Class *dcc = g_dcf->get_class_by_id(dc_id);
			if(!dcc)
			{
				m_log->error() << "Received create_object with unknown dclass '"
				               << dc_id << "'.\n";
				resp->add_doid(INVALID_DO_ID);
				route_datagram(resp);
				return;
			}

			// Unpack fields to be passed to database
			ObjectData dbo(dc_id);
			uint16_t field_count = dgi.read_uint16();
			m_log->trace() << "Unpacking fields..." << endl;
			try
			{
				for(uint16_t i = 0; i < field_count; ++i)
				{
					uint16_t field_id = dgi.read_uint16();
					const Field *field = dcc->get_field_by_id(field_id);
					if(field)
					{
						if(field->has_keyword("db"))
						{
							dgi.unpack_field(field, dbo.fields[field]);
						}
						else
						{
							m_log->warning() << "Recieved non-db field in create_object.\n";
							dgi.skip_field(field);
						}
					}
				}
			}
			catch(exception &e)
			{
				m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
				               << e.what() << endl;

				resp->add_doid(INVALID_DO_ID);
				route_datagram(resp);
				return;
			}

			// Populate with defaults
			m_log->trace() << "Checking all required fields exist..." << endl;
			for(unsigned int i = 0; i < dcc->get_num_fields(); ++i)
			{
				const Field *field = dcc->get_field(i);
				if(field->has_keyword("db") && !field->as_molecular()
				        && dbo.fields.find(field) == dbo.fields.end() && field->has_default_value())
				{
                    string val = field->get_default_value();
                    dbo.fields[field] = vector<uint8_t>(val.begin(), val.end());
				}
			}

			// Create object in database
			m_log->trace() << "Creating stored object..." << endl;
			doid_t do_id = m_db_backend->create_object(dbo);
			if(do_id == INVALID_DO_ID || do_id < m_min_id || do_id > m_max_id)
			{
				m_log->error() << "Ran out of DistributedObject ids while creating new object." << endl;
				resp->add_doid(INVALID_DO_ID);
				route_datagram(resp);
				return;
			}

			// Send the newly allocated doid back to caller
			m_log->trace() << "... created with ID: " << do_id << endl;
			resp->add_doid(do_id);
			route_datagram(resp);
		}
		break;
		case DBSERVER_OBJECT_GET_ALL:
		{
			uint32_t context = dgi.read_uint32();

			// Start the reply datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_ALL_RESP);
			resp->add_uint32(context);

			doid_t do_id = dgi.read_doid();

			m_log->trace() << "Selecting all from do_id: " << do_id << "... " << endl;

			// Get object from database
			ObjectData dbo;
			if(m_db_backend->get_object(do_id, dbo))
			{
				// Object found, serialize fields and send all data back
				m_log->trace() << "... object found!" << endl;
				resp->add_uint8(SUCCESS);
				resp->add_uint16(dbo.dc_id);
				resp->add_uint16(dbo.fields.size());
				for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
				{
					resp->add_uint16(it->first->get_id());
					resp->add_data(it->second);
					m_log->trace() << "Recieved field id-" << it->first->get_id() << ", value-"
					               << string(it->second.begin(), it->second.end()) << "\n";
				}
			}
			else
			{
				// Object not found, send failure.
				m_log->trace() << "... object not found." << endl;
				resp->add_uint8(FAILURE);
			}

			// Send reply datagram
			route_datagram(resp);
		}
		break;
		case DBSERVER_OBJECT_DELETE:
		{
			// Just delete the object
			doid_t do_id = dgi.read_doid();
			m_db_backend->delete_object(do_id);

			m_log->debug() << "Deleted object with ID: " << do_id << endl;

			// Broadcast update to object's channel
			if(m_broadcast)
			{
				DatagramPtr update = Datagram::create();
				update->add_server_header(DATABASE2OBJECT(do_id), sender, DBSERVER_OBJECT_DELETE);
				update->add_doid(do_id);
				route_datagram(update);
			}
		}
		break;
		case DBSERVER_OBJECT_SET_FIELD:
		{
			// Check class so we can filter the field
			doid_t do_id = dgi.read_doid();
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				return;
			}

			uint16_t field_id = dgi.read_uint16();
			const Field *field = dcc->get_field_by_id(field_id);

			// Make sure field exists and is a database field
			if(!field || !field->has_keyword("db"))
			{
				return;
			}

			// Update the field value
			vector<uint8_t> value;
			dgi.unpack_field(field, value);
			m_db_backend->set_field(do_id, field, value);

			// Broadcast update to object's channel
			if(m_broadcast)
			{
				DatagramPtr update = Datagram::create();
				update->add_server_header(DATABASE2OBJECT(do_id), sender, DBSERVER_OBJECT_SET_FIELD);
				update->add_doid(do_id);
				update->add_uint16(field_id);
				update->add_data(value);
				route_datagram(update);
			}
		}
		break;
		case DBSERVER_OBJECT_SET_FIELDS:
		{
			// Check class so we can filter the fieldss
			doid_t do_id = dgi.read_doid();
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				return;
			}

			// Make sure field exists and are database fields, storing their values
			map<const Field*, vector<uint8_t> > fields;
			uint16_t field_count = dgi.read_uint16();
			m_log->trace() << "Unpacking fields..." << endl;
			try
			{
				for(uint16_t i = 0; i < field_count; ++i)
				{
					uint16_t field_id = dgi.read_uint16();
					const Field *field = dcc->get_field_by_id(field_id);
					if(field)
					{
						if(field->has_keyword("db"))
						{
							dgi.unpack_field(field, fields[field]);
						}
						else
						{
							dgi.skip_field(field);
						}
					}
				}
			}
			catch(exception &e)
			{
				m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
				               << e.what() << endl;
				return;
			}

			// Update the field values in the database
			m_db_backend->set_fields(do_id, fields);

			if(m_broadcast)
			{
				// Broadcast update to object's channel
				DatagramPtr update = Datagram::create();
				update->add_server_header(DATABASE2OBJECT(do_id), sender,
				                         DBSERVER_OBJECT_SET_FIELDS);

				// Seek to doid & field-data and copy it to update
				dgi.seek_payload();
				dgi.skip(sizeof(channel_t) + sizeof(uint16_t));
				update->add_data(dgi.read_remainder());
				route_datagram(update);
			}
		}
		break;
		case DBSERVER_OBJECT_SET_FIELD_IF_EMPTY:
		{
			uint32_t context = dgi.read_uint32();

			// Start response datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel,
			                       DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP);
			resp->add_uint32(context);

			// Get class so we can filter the field
			doid_t do_id = dgi.read_doid();
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Verify the field is a database field
			uint16_t field_id = dgi.read_uint16();
			const Field *field = dcc->get_field_by_id(field_id);
			if(!field->has_keyword("db"))
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Try to set the field
			vector<uint8_t> value;
			dgi.unpack_field(field, value);
			if(m_db_backend->set_field_if_empty(do_id, field, value))
			{
				// Update was successful, send reply
				resp->add_uint8(SUCCESS);
				route_datagram(resp);

				if(m_broadcast)
				{
					DatagramPtr update = Datagram::create();
					update->add_server_header(DATABASE2OBJECT(do_id), sender,
					                         DBSERVER_OBJECT_SET_FIELD);
					update->add_doid(do_id);
					update->add_uint16(field_id);
					update->add_data(value);
					route_datagram(update);
				}
				return;
			}

			// Update failed, send back current object values
			resp->add_uint8(FAILURE);
			if(value.size() > 0)
			{
				resp->add_uint16(field_id);
				resp->add_data(value);
			}
			route_datagram(resp);
		}
		break;
		case DBSERVER_OBJECT_SET_FIELD_IF_EQUALS:
		{
			uint32_t context = dgi.read_uint32();

			// Start response datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel,
			                       DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP);
			resp->add_uint32(context);

			// Get class so we can filter the field
			doid_t do_id = dgi.read_doid();
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Verify the field is a database field
			uint16_t field_id = dgi.read_uint16();
			const Field *field = dcc->get_field_by_id(field_id);
			if(!field->has_keyword("db"))
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			vector<uint8_t> equal;
			vector<uint8_t> value;
			dgi.unpack_field(field, equal);
			dgi.unpack_field(field, value);

			// Try to set the field
			if(m_db_backend->set_field_if_equals(do_id, field, equal, value))
			{
				// Update was successful, send reply
				resp->add_uint8(SUCCESS);
				route_datagram(resp);

				if(m_broadcast)
				{
					DatagramPtr update = Datagram::create();
					update->add_server_header(DATABASE2OBJECT(do_id), sender,
					                         DBSERVER_OBJECT_SET_FIELD);
					update->add_doid(do_id);
					update->add_uint16(field_id);
					update->add_data(value);
					route_datagram(update);
				}
				return;
			}

			// Update failed, send back current object values
			resp->add_uint8(FAILURE);
			if(value.size() > 0)
			{
				resp->add_uint16(field_id);
				resp->add_data(value);
			}
			route_datagram(resp);
		}
		break;
		case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS:
		{
			uint32_t context = dgi.read_uint32();

			// Start response datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel,
			                       DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP);
			resp->add_uint32(context);

			// Get class so we can filter the fields
			doid_t do_id = dgi.read_doid();
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Unpack fields from datagram, validate that they are database fields
			map<const Field*, vector<uint8_t> > equals;
			map<const Field*, vector<uint8_t> > values;

			// We're going to use this to track the order locally, updates are handled in order
			// already for the database because later updates overwrite newer updates.
			list<const Field*> ordered;

			uint16_t field_count = dgi.read_uint16();
			try
			{
				for(uint16_t i = 0; i < field_count; ++i)
				{
					uint16_t field_id = dgi.read_uint16();
					const Field *field = dcc->get_field_by_id(field_id);
					if(field)
					{
						if(field->has_keyword("db"))
						{
							ordered.push_back(field);
							dgi.unpack_field(field, equals[field]);
							dgi.unpack_field(field, values[field]);
						}
						else
						{
							resp->add_uint8(FAILURE);
							route_datagram(resp);
							return;
						}
					}
				}
			}
			catch(exception &e)
			{
				m_log->error() << "Error while unpacking fields, msg may be truncated."
				               << " e.what(): " << e.what() << endl;
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Try to update the values
			if(m_db_backend->set_fields_if_equals(do_id, equals, values))
			{
				// Update was successful, send reply
				resp->add_uint8(SUCCESS);
				route_datagram(resp);

				if(m_broadcast)
				{
					// Broadcast update to object's channel
					DatagramPtr update = Datagram::create();
					update->add_server_header(DATABASE2OBJECT(do_id), sender,
					                         DBSERVER_OBJECT_SET_FIELDS);
					update->add_doid(do_id);
					update->add_uint16(field_count);
					for(auto it = ordered.begin(); it != ordered.end(); ++it)
					{
						const Field* field = *it;
						update->add_uint16(field->get_id());
						update->add_data(values[field]);
					}
					route_datagram(update);
				}
				return;
			}

			// Update failed, send back current object values
			resp->add_uint8(FAILURE);
			if(values.size() > 0)
			{
				resp->add_uint16(values.size());
				for(auto it = values.begin(); it != values.end(); ++it)
				{
					resp->add_uint16(it->first->get_id());
					resp->add_data(it->second);
				}
			}
			route_datagram(resp);
		}
		break;
		case DBSERVER_OBJECT_GET_FIELD:
		{
			uint32_t context = dgi.read_uint32();

			// Start response datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELD_RESP);
			resp->add_uint32(context);

			// Get object id from datagram
			doid_t do_id = dgi.read_doid();
			m_log->trace() << "Reading field from obj-" << do_id << "..." << endl;

			// Get object class from db
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				m_log->trace() << "... object not found in database." << endl;
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Get field from datagram
			uint16_t field_id = dgi.read_uint16();
			const Field *field = dcc->get_field_by_id(field_id);
			if(!field)
			{
				m_log->error() << "Asked for invalid field,"
				               << " reading from obj-" << do_id << endl;
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Get value from database
			vector<uint8_t> value;
			if(!m_db_backend->get_field(do_id, field, value))
			{
				m_log->trace() << "... field not set." << endl;
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Send value in response
			resp->add_uint8(SUCCESS);
			resp->add_uint16(field_id);
			resp->add_data(value);
			route_datagram(resp);
			m_log->trace() << "... success." << endl;
		}
		break;
		case DBSERVER_OBJECT_GET_FIELDS:
		{
			uint32_t context = dgi.read_uint32();

			// Start response datagram
			DatagramPtr resp = Datagram::create();
			resp->add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELDS_RESP);
			resp->add_uint32(context);

			// Get object id from datagram
			doid_t do_id = dgi.read_doid();
			m_log->trace() << "Reading fields from obj-" << do_id << "..." << endl;

			// Get object class from db
			const Class *dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Get fields from datagram
			uint16_t field_count = dgi.read_uint16();
			vector<const Field*> fields(field_count);
			for(uint16_t i = 0; i < field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				const Field *field = dcc->get_field_by_id(field_id);
				if(!field)
				{
					m_log->error() << "Asked for invalid field(s),"
					               << " reading from object #" << do_id << endl;
					resp->add_uint8(FAILURE);
					route_datagram(resp);
					return;
				}
				fields[i] = field;
			}

			// Get values from database
			map<const Field*, vector<uint8_t> > values;
			if(!m_db_backend->get_fields(do_id, fields, values))
			{
				m_log->trace() << "... failure." << endl;
				resp->add_uint8(FAILURE);
				route_datagram(resp);
				return;
			}

			// Send values in response
			resp->add_uint8(SUCCESS);
			resp->add_uint16(values.size());
			for(auto it = values.begin(); it != values.end(); ++it)
			{
				resp->add_uint16(it->first->get_id());
				resp->add_data(it->second);
			}
			route_datagram(resp);
			m_log->trace() << "... success." << endl;
		}
		break;
		case DBSERVER_OBJECT_DELETE_FIELD:
		{
			doid_t do_id = dgi.read_doid();
			m_log->trace() << "Deleting field of obj-" << do_id << "..." << endl;

			// Get class so we can validate the field
			const Class* dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				m_log->trace() << "... object does not exist." << endl;
				return;
			}

			// Check the field exists
			uint16_t field_id = dgi.read_uint16();
			const Field* field = dcc->get_field_by_id(field_id);
			if(!field)
			{
				m_log->error() << "Tried to delete an invalid field:" << field_id
				               << " on obj-" << do_id << "." << endl;
				return;
			}

			// Check the field is a database field
			if(field->has_keyword("db"))
			{
				// If field has a default, use it
				if(field->has_default_value())
				{
					string str = field->get_default_value();
					vector<uint8_t> value(str.begin(), str.end());
					/* TODO: Alternate implementation for performance compare */
					//vector<uint8_t> value(str.length());
					//memcpy(&value[0], str.c_str(), str.length());
					m_db_backend->set_field(do_id, field, value);

					m_log->trace() << "... field set to default.\n";

					if(m_broadcast)
					{
						DatagramPtr update = Datagram::create();
						update->add_server_header(DATABASE2OBJECT(do_id), sender,
						                         DBSERVER_OBJECT_SET_FIELD);
						update->add_doid(do_id);
						update->add_uint16(field_id);
						update->add_data(value);
						route_datagram(update);
					}
				}

				// Otherwise just delete the field
				else
				{
					m_db_backend->del_field(do_id, field);

					m_log->trace() << "... field deleted.\n";

					if(m_broadcast)
					{
						DatagramPtr update = Datagram::create();
						update->add_server_header(DATABASE2OBJECT(do_id), sender,
						                         DBSERVER_OBJECT_DELETE_FIELD);
						update->add_doid(do_id);
						update->add_uint16(field_id);
						route_datagram(update);
					}
				}
			}
			else
			{
				m_log->warning() << "Cannot delete non-db field of obj-" << do_id
				                 << "." << endl;
			}
		}
		break;
		case DBSERVER_OBJECT_DELETE_FIELDS:
		{
			doid_t do_id = dgi.read_doid();
			m_log->trace() << "Deleting field of obj-" << do_id << "..." << endl;

			// Get class to validate fields
			const Class* dcc = m_db_backend->get_class(do_id);
			if(!dcc)
			{
				m_log->trace() << "... object does not exist." << endl;
				return;
			}

			vector<const Field*> del_fields; // deleted fields
			map<const Field*, vector<uint8_t> > set_fields; // set to default

			// Iterate through all the fields
			uint16_t field_count = dgi.read_uint16();
			for(uint16_t i = 0; i < field_count; ++i)
			{
				// Check that field actually exists
				uint16_t field_id = dgi.read_uint16();
				const Field* field = dcc->get_field_by_id(field_id);
				if(!field)
				{
					m_log->error() << "Tried to delete an invalid field:" << field_id
					               << " on obj-" << do_id << "." << endl;
					return;
				}

				// Check the field is actually a database field
				if(field->has_keyword("db"))
				{
					// If field has a default, use it
					if(field->has_default_value())
					{
						string str = field->get_default_value();
						vector<uint8_t> value(str.begin(), str.end());
						/* Alternate implementation for performance compare */
						//vector<uint8_t> value(str.length());
						//memcpy(&value[0], str.c_str(), str.length());
						set_fields[field] = value;
						m_log->trace() << "... field set to default ..." << endl;
					}

					// Otherwise just delete the field
					else
					{
						del_fields.push_back(field);
						m_log->trace() << "... field deleted ..." << endl;
					}
				}
				else
				{
					m_log->warning() << "Cannot delete non-db field of obj-" << do_id
					                 << "." << endl;
				}
			}

			// Delete fields marked for deltion
			if(del_fields.size() > 0)
			{
				m_db_backend->del_fields(do_id, del_fields);

				m_log->trace() << "... fields deleted.\n";

				if(m_broadcast)
				{
					DatagramPtr update = Datagram::create();
					update->add_server_header(DATABASE2OBJECT(do_id), sender,
					                         DBSERVER_OBJECT_DELETE_FIELDS);
					update->add_doid(do_id);
					update->add_uint16(del_fields.size());
					for(auto it = del_fields.begin(); it != del_fields.end(); ++it)
					{
						update->add_uint16((*it)->get_id());
					}
					route_datagram(update);
				}
			}

			// Update fields with default values
			if(set_fields.size() > 0)
			{
				m_db_backend->set_fields(do_id, set_fields);

				m_log->trace() << "... fields deleted.\n";

				if(m_broadcast)
				{
					DatagramPtr update = Datagram::create();
					update->add_server_header(DATABASE2OBJECT(do_id), sender,
					                         DBSERVER_OBJECT_SET_FIELDS);
					update->add_doid(do_id);
					update->add_uint16(set_fields.size());
					for(auto it = set_fields.begin(); it != set_fields.end(); ++it)
					{
						update->add_uint16(it->first->get_id());
						update->add_data(it->second);
					}
					route_datagram(update);
				}
			}
		}
		break;
		default:
			m_log->error() << "Recieved unknown MsgType: " << msg_type << endl;
	};
}
