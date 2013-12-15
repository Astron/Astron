#include "core/global.h"
#include "core/msgtypes.h"
#include "core/RoleFactory.h"
#include "DBBackendFactory.h"
#include "DatabaseBackend.h"

using dclass::Field;
using dclass::Class;

static ConfigVariable<channel_t> control_channel("control", INVALID_CHANNEL);
static ConfigVariable<doid_t> min_id("generate/min", INVALID_DO_ID);
static ConfigVariable<doid_t> max_id("generate/max", UINT_MAX);
static ConfigVariable<std::string> database_type("backend/type", "yaml");

class DatabaseServer : public Role
{
	private:
		DatabaseBackend *m_db_backend;
		LogCategory *m_log;

		channel_t m_control_channel;
		doid_t m_min_id, m_max_id;
	public:
		DatabaseServer(RoleConfig roleconfig) : Role(roleconfig),
			m_db_backend(DBBackendFactory::singleton.instantiate_backend(
			                database_type.get_rval(roleconfig),
			                roleconfig["backend"],
			                min_id.get_rval(roleconfig),
			                max_id.get_rval(roleconfig))),
			m_control_channel(control_channel.get_rval(roleconfig)),
			m_min_id(min_id.get_rval(roleconfig)),
			m_max_id(max_id.get_rval(roleconfig))
		{
			// Initialize DatabaseServer log
			std::stringstream log_title;
			log_title << "Database(" << m_control_channel << ")";
			m_log = new LogCategory("db", log_title.str());
			set_con_name(log_title.str());

			// Check to see the backend was instantiated
			if(!m_db_backend)
			{
				m_log->fatal() << "No database backend of type '"
				               << database_type.get_rval(roleconfig) << "' exists." << std::endl;
				exit(1);
			}

			// Listen on control channel
			subscribe_channel(m_control_channel);
		}

		virtual void handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
		{
			channel_t sender = dgi.read_channel();
			uint16_t msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case DBSERVER_CREATE_OBJECT:
				{
					uint32_t context = dgi.read_uint32();

					// Start response with generic header
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_CREATE_OBJECT_RESP);
					resp.add_uint32(context);

					// Get DistributedClass
					uint16_t dc_id = dgi.read_uint16();
					Class *dcc = g_dcf->get_class(dc_id);
					if(!dcc)
					{
						m_log->error() << "Invalid Class when creating object: #" << dc_id << std::endl;
						resp.add_doid(INVALID_DO_ID);
						route_datagram(resp);
						return;
					}

					// Unpack fields to be passed to database
					ObjectData dbo(dc_id);
					uint16_t field_count = dgi.read_uint16();
					m_log->trace() << "Unpacking fields..." << std::endl;
					try
					{
						for(uint16_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							Field *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
								{
									dgi.unpack_field(field, dbo.fields[field]);
								}
								else
								{
									m_log->warning() << "Recieved non-db field in CREATE_STORED_OBJECT." << std::endl;
									dgi.skip_field(field);
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
						               << e.what() << std::endl;

						resp.add_doid(INVALID_DO_ID);
						route_datagram(resp);
						return;
					}

					// Populate with defaults
					m_log->trace() << "Checking all required fields exist..." << std::endl;
					for(int i = 0; i < dcc->get_num_inherited_fields(); ++i)
					{
						Field *field = dcc->get_inherited_field(i);
						if(field->is_db() && !field->as_molecular_field()
						        && dbo.fields.find(field) == dbo.fields.end() && field->has_default_value())
						{
                            std::string val = field->get_default_value();
                            dbo.fields[field] = std::vector<uint8_t>(val.begin(), val.end());
						}
					}

					// Create object in database
					m_log->trace() << "Creating stored object..." << std::endl;
					doid_t do_id = m_db_backend->create_object(dbo);
					if(do_id == INVALID_DO_ID || do_id < m_min_id || do_id > m_max_id)
					{
						m_log->error() << "Ran out of DistributedObject ids while creating new object." << std::endl;
						resp.add_doid(INVALID_DO_ID);
						route_datagram(resp);
						return;
					}

					// Send the newly allocated doid back to caller
					m_log->trace() << "... created with ID: " << do_id << std::endl;
					resp.add_doid(do_id);
					route_datagram(resp);
				}
				break;
				case DBSERVER_OBJECT_GET_ALL:
				{
					uint32_t context = dgi.read_uint32();

					// Start the reply datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_ALL_RESP);
					resp.add_uint32(context);

					doid_t do_id = dgi.read_doid();

					m_log->trace() << "Selecting all from do_id: " << do_id << "... " << std::endl;

					// Get object from database
					ObjectData dbo;
					if(m_db_backend->get_object(do_id, dbo))
					{
						// Object found, serialize fields and send all data back
						m_log->trace() << "... object found!" << std::endl;
						resp.add_uint8(SUCCESS);
						resp.add_uint16(dbo.dc_id);
						resp.add_uint16(dbo.fields.size());
						for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
						{
							resp.add_uint16(it->first->get_number());
							resp.add_data(it->second);
							m_log->trace() << "Recieved field id-" << it->first->get_number() << ", value-" << std::string(
							                  it->second.begin(), it->second.end()) << std::endl;
						}
					}
					else
					{
						// Object not found, send failure.
						m_log->trace() << "... object not found." << std::endl;
						resp.add_uint8(FAILURE);
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
					m_log->debug() << "Deleted object with ID: " << do_id << std::endl;
				}
				break;
				case DBSERVER_OBJECT_SET_FIELD:
				{
					// Check class so we can filter the field
					doid_t do_id = dgi.read_doid();
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						return;
					}

					uint16_t field_id = dgi.read_uint16();
					Field *field = dcc->get_field_by_index(field_id);

					// Make sure field exists and is a database field
					if(field && field->is_db())
					{
						// Update the field value
						std::vector<uint8_t> value;
						dgi.unpack_field(field, value);
						m_db_backend->set_field(do_id, field, value);
					}
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS:
				{
					// Check class so we can filter the fieldss
					doid_t do_id = dgi.read_doid();
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						return;
					}

					// Make sure field exists and are database fields, storing their values
					std::map<Field*, std::vector<uint8_t> > fields;
					uint16_t field_count = dgi.read_uint16();
					m_log->trace() << "Unpacking fields..." << std::endl;
					try
					{
						for(uint16_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							Field *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
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
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated. e.what(): "
						               << e.what() << std::endl;
						return;
					}

					// Update the field values in the database
					m_db_backend->set_fields(do_id, fields);
				}
				break;
				case DBSERVER_OBJECT_SET_FIELD_IF_EMPTY:
				{
					uint32_t context = dgi.read_uint32();

					// Start response datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP);
					resp.add_uint32(context);

					// Get class so we can filter the field
					doid_t do_id = dgi.read_doid();
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Verify the field is a database field
					uint16_t field_id = dgi.read_uint16();
					Field *field = dcc->get_field_by_index(field_id);
					if(!field->is_db())
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Try to set the field
					std::vector<uint8_t> value;
					dgi.unpack_field(field, value);
					if(m_db_backend->set_field_if_empty(do_id, field, value))
					{
						// Update was successful, send reply
						resp.add_uint8(SUCCESS);
						route_datagram(resp);
						return;
					}

					// Update failed, send back current object values
					resp.add_uint8(FAILURE);
					if(value.size() > 0)
					{
						resp.add_uint16(field_id);
						resp.add_data(value);
					}
					route_datagram(resp);
				}
				break;
				case DBSERVER_OBJECT_SET_FIELD_IF_EQUALS:
				{
					uint32_t context = dgi.read_uint32();

					// Start response datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP);
					resp.add_uint32(context);

					// Get class so we can filter the field
					doid_t do_id = dgi.read_doid();
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Verify the field is a database field
					uint16_t field_id = dgi.read_uint16();
					Field *field = dcc->get_field_by_index(field_id);
					if(!field->is_db())
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					std::vector<uint8_t> equal;
					std::vector<uint8_t> value;
					dgi.unpack_field(field, equal);
					dgi.unpack_field(field, value);

					// Try to set the field
					if(m_db_backend->set_field_if_equals(do_id, field, equal, value))
					{
						// Update was successful, send reply
						resp.add_uint8(SUCCESS);
						route_datagram(resp);
						return;
					}

					// Update failed, send back current object values
					resp.add_uint8(FAILURE);
					if(value.size() > 0)
					{
						resp.add_uint16(field_id);
						resp.add_data(value);
					}
					route_datagram(resp);
				}
				break;
				case DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS:
				{
					uint32_t context = dgi.read_uint32();

					// Start response datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP);
					resp.add_uint32(context);

					// Get class so we can filter the fields
					doid_t do_id = dgi.read_doid();
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Unpack fields from datagram, validate that they are database fields
					std::map<Field*, std::vector<uint8_t> > equals;
					std::map<Field*, std::vector<uint8_t> > values;
					uint16_t field_count = dgi.read_uint16();
					try
					{
						for(uint16_t i = 0; i < field_count; ++i)
						{
							uint16_t field_id = dgi.read_uint16();
							Field *field = dcc->get_field_by_index(field_id);
							if(field)
							{
								if(field->is_db())
								{
									dgi.unpack_field(field, equals[field]);
									dgi.unpack_field(field, values[field]);
								}
								else
								{
									resp.add_uint8(FAILURE);
									route_datagram(resp);
									return;
								}
							}
						}
					}
					catch(std::exception &e)
					{
						m_log->error() << "Error while unpacking fields, msg may be truncated."
						               << " e.what(): " << e.what() << std::endl;
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Try to update the values
					if(m_db_backend->set_fields_if_equals(do_id, equals, values))
					{
						// Update was successful, send reply
						resp.add_uint8(SUCCESS);
						route_datagram(resp);
						return;
					}

					// Update failed, send back current object values
					resp.add_uint8(FAILURE);
					if(values.size() > 0)
					{
						resp.add_uint16(values.size());
						for(auto it = values.begin(); it != values.end(); ++it)
						{
							resp.add_uint16(it->first->get_number());
							resp.add_data(it->second);
						}
					}
					route_datagram(resp);
				}
				break;
				case DBSERVER_OBJECT_GET_FIELD:
				{
					uint32_t context = dgi.read_uint32();

					// Start response datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELD_RESP);
					resp.add_uint32(context);

					// Get object id from datagram
					doid_t do_id = dgi.read_doid();
					m_log->trace() << "Reading field from obj-" << do_id << "..." << std::endl;

					// Get object class from db
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						m_log->trace() << "... object not found in database." << std::endl;
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Get field from datagram
					uint16_t field_id = dgi.read_uint16();
					Field *field = dcc->get_field_by_index(field_id);
					if(!field)
					{
						m_log->error() << "Asked for invalid field,"
						               << " reading from obj-" << do_id << std::endl;
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Get value from database
					std::vector<uint8_t> value;
					if(!m_db_backend->get_field(do_id, field, value))
					{
						m_log->trace() << "... field not set." << std::endl;
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Send value in response
					resp.add_uint8(SUCCESS);
					resp.add_uint16(field_id);
					resp.add_data(value);
					route_datagram(resp);
					m_log->trace() << "... success." << std::endl;
				}
				break;
				case DBSERVER_OBJECT_GET_FIELDS:
				{
					uint32_t context = dgi.read_uint32();

					// Start response datagram
					Datagram resp;
					resp.add_server_header(sender, m_control_channel, DBSERVER_OBJECT_GET_FIELDS_RESP);
					resp.add_uint32(context);

					// Get object id from datagram
					doid_t do_id = dgi.read_doid();
					m_log->trace() << "Reading fields from obj-" << do_id << "..." << std::endl;

					// Get object class from db
					Class *dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Get fields from datagram
					uint16_t field_count = dgi.read_uint16();
					std::vector<Field*> fields(field_count);
					for(uint16_t i = 0; i < field_count; ++i)
					{
						uint16_t field_id = dgi.read_uint16();
						Field *field = dcc->get_field_by_index(field_id);
						if(!field)
						{
							m_log->error() << "Asked for invalid field(s),"
							               << " reading from object #" << do_id << std::endl;
							resp.add_uint8(FAILURE);
							route_datagram(resp);
							return;
						}
						fields[i] = field;
					}

					// Get values from database
					std::map<Field*, std::vector<uint8_t> > values;
					if(!m_db_backend->get_fields(do_id, fields, values))
					{
						m_log->trace() << "... failure." << std::endl;
						resp.add_uint8(FAILURE);
						route_datagram(resp);
						return;
					}

					// Send values in response
					resp.add_uint8(SUCCESS);
					resp.add_uint16(values.size());
					for(auto it = values.begin(); it != values.end(); ++it)
					{
						resp.add_uint16(it->first->get_number());
						resp.add_data(it->second);
					}
					route_datagram(resp);
					m_log->trace() << "... success." << std::endl;
				}
				break;
				case DBSERVER_OBJECT_DELETE_FIELD:
				{
					doid_t do_id = dgi.read_doid();
					m_log->trace() << "Deleting field of obj-" << do_id << "..." << std::endl;

					// Get class so we can validate the field
					Class* dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						m_log->trace() << "... object does not exist." << std::endl;
						return;
					}

					// Check the field exists
					uint16_t field_id = dgi.read_uint16();
					Field* field = dcc->get_field_by_index(field_id);
					if(!field)
					{
						m_log->error() << "Tried to delete an invalid field:" << field_id
						               << " on obj-" << do_id << "." << std::endl;
						return;
					}

					// Check the field is a database field
					if(field->is_db())
					{
						// If field has a default, use it
						if(field->has_default_value())
						{
							std::string str = field->get_default_value();
							std::vector<uint8_t> value(str.begin(), str.end());
							/* Alternate implementation for performance compare */
							//std::vector<uint8_t> value(str.length());
							//memcpy(&value[0], str.c_str(), str.length());
							m_db_backend->set_field(do_id, field, value);
							m_log->trace() << "... field set to default." << std::endl;
						}

						// Otherwise just delete the field
						else
						{
							m_db_backend->del_field(do_id, field);
							m_log->trace() << "... field deleted." << std::endl;
						}
					}
					else
					{
						m_log->warning() << "Cannot delete non-db field of obj-" << do_id
						                 << "." << std::endl;
					}
				}
				break;
				case DBSERVER_OBJECT_DELETE_FIELDS:
				{
					doid_t do_id = dgi.read_doid();
					m_log->trace() << "Deleting field of obj-" << do_id << "..." << std::endl;

					// Get class to validate fields
					Class* dcc = m_db_backend->get_class(do_id);
					if(!dcc)
					{
						m_log->trace() << "... object does not exist." << std::endl;
						return;
					}

					std::vector<Field*> del_fields; // deleted fields
					std::map<Field*, std::vector<uint8_t> > set_fields; // set to default

					// Iterate through all the fields
					uint16_t field_count = dgi.read_uint16();
					for(uint16_t i = 0; i < field_count; ++i)
					{
						// Check that field actually exists
						uint16_t field_id = dgi.read_uint16();
						Field* field = dcc->get_field_by_index(field_id);
						if(!field)
						{
							m_log->error() << "Tried to delete an invalid field:" << field_id
							               << " on obj-" << do_id << "." << std::endl;
							return;
						}

						// Check the field is actually a database field
						if(field->is_db())
						{
							// If field has a default, use it
							if(field->has_default_value())
							{
								std::string str = field->get_default_value();
								std::vector<uint8_t> value(str.begin(), str.end());
								/* Alternate implementation for performance compare */
								//std::vector<uint8_t> value(str.length());
								//memcpy(&value[0], str.c_str(), str.length());
								set_fields[field] = value;
								m_log->trace() << "... field set to default ..." << std::endl;
							}

							// Otherwise just delete the field
							else
							{
								del_fields.push_back(field);
								m_log->trace() << "... field deleted ..." << std::endl;
							}
						}
						else
						{
							m_log->warning() << "Cannot delete non-db field of obj-" << do_id
							                 << "." << std::endl;
						}
					}

					// Delete fields marked for deltion
					if(del_fields.size() > 0)
					{
						m_db_backend->del_fields(do_id, del_fields);
						m_log->trace() << "... fields deleted." << std::endl;
					}

					// Update fields with default values
					if(set_fields.size() > 0)
					{
						m_db_backend->set_fields(do_id, set_fields);
						m_log->trace() << "... fields deleted." << std::endl;
					}
				}
				break;
				default:
					m_log->error() << "Recieved unknown MsgType: " << msg_type << std::endl;
			};
		}

};

RoleFactoryItem<DatabaseServer> dbserver_fact("database");
