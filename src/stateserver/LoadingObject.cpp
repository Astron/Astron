#include "core/global.h"

#include "LoadingObject.h"

uint32_t LoadingObject::cls_next_context = 0;

LoadingObject::LoadingObject(DBStateServer *stateserver, uint32_t do_id,
                             uint32_t parent_id, uint32_t zone_id) :
	m_dbss(stateserver), m_do_id(do_id), m_parent_id(parent_id), m_zone_id(zone_id)
{
	std::stringstream name;
	name << "LoadingObject(doid: " << do_id << ", db: " << m_dbss->m_db_channel << ")";
	m_log = new LogCategory("dbobject", name.str());

	MessageDirector::singleton.subscribe_channel(this, do_id);

	send_get_object(do_id);
}

LoadingObject::LoadingObject(DBStateServer *stateserver, uint32_t do_id, uint32_t parent_id,
                             uint32_t zone_id, DCClass *dclass, DatagramIterator &dgi) :
	m_dbss(stateserver), m_do_id(do_id), m_parent_id(parent_id), m_zone_id(zone_id), m_dclass(dclass)
{
	// TODO: Implement
}

void LoadingObject::send_get_object(uint32_t do_id)
{
	m_context = cls_next_context++;

	Datagram dg(m_dbss->m_db_channel, do_id, DBSERVER_OBJECT_GET_ALL);
	dg.add_uint32(m_context); // Context
	dg.add_uint32(do_id);
	send(dg);
}

void LoadingObject::replay_datagrams(DistributedObject* obj)
{
	for(auto it = m_datagram_queue.begin(); it != m_datagram_queue.end(); ++it)
	{
		try
		{
			DatagramIterator dgi(*it);
			dgi.seek_payload();
			obj->handle_datagram(*it, dgi);

			dgi.seek_payload();
			m_dbss->handle_datagram(*it, dgi);
		}
		catch(DatagramIteratorEOF &e)
		{
			m_log->error() << "Detected truncated datagram in while replaying"
			               " datagrams to object and dbss. Skipped." << std::endl;
		}
	}
}

void LoadingObject::handle_datagram(Datagram &in_dg, DatagramIterator &dgi)
{
	channel_t sender = dgi.read_uint64();
	uint16_t msgtype = dgi.read_uint16();
	switch(msgtype)
	{
		case DBSERVER_OBJECT_GET_ALL_RESP:
		{
			if(dgi.read_uint32() != m_context)
			{
				m_log->warning() << "Received get_all_resp with incorrect context" << std::endl;
				break;
			}

			if(dgi.read_uint8() != true)
			{
				m_log->debug() << "Object not found in database." << std::endl;
				m_dbss->discard_loader(m_do_id);
				break;
			}

			uint16_t dc_id = dgi.read_uint32();
			if(dc_id >= g_dcf->get_num_classes())
			{
				m_log->error() << "Received object from database with unknown dclass"
				               << " - id:" << dc_id << std::endl;
				m_dbss->discard_loader(m_do_id);
				break;
			}

			DCClass *r_dclass = g_dcf->get_class(dc_id);
			if(m_dclass && r_dclass != m_dclass)
			{
				m_log->error() << "Requested object of class " << m_dclass->get_number()
				               << ", but received class " << dc_id << std::endl;
				m_dbss->discard_loader(m_do_id);
				break;
			}

			// Get fields from database
			uint16_t db_field_count = dgi.read_uint16();
			for(uint16_t i = 0; i < db_field_count; ++i)
			{
				uint16_t field_id = dgi.read_uint16();
				DCField *field = r_dclass->get_field_by_index(field_id);
				if(field->is_ram())
				{
					dgi.unpack_field(field, m_ram_fields[field]);
				}
				else if(field->is_required())
				{
					dgi.unpack_field(field, m_required_fields[field]);
				}
				else
				{
					dgi.skip_field(field);
				}
			}

			// Add default values and updated values
			int dcc_field_count = r_dclass->get_num_inherited_fields();
			for(int i = 0; i < dcc_field_count; ++i)
			{
				DCField *field = m_dclass->get_inherited_field(i);
				if(!field->as_molecular_field())
				{
					if(field->is_required())
					{
						if(m_field_updates.find(field) != m_field_updates.end())
						{
							m_required_fields[field] = m_field_updates[field];
						}
						else
						{
							std::string val = field->get_default_value();
							m_required_fields[field] = std::vector<uint8_t>(val.begin(), val.end());
						}
					}
					else if(field->is_ram())
					{
						if(m_field_updates.find(field) != m_field_updates.end())
						{
							m_ram_fields[field] = m_field_updates[field];
						}
						else
						{
							std::string val = field->get_default_value();
							m_ram_fields[field] = std::vector<uint8_t>(val.begin(), val.end());
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
			m_dbss->discard_loader(m_do_id);
			break;
		}
		default:
		{
			m_datagram_queue.push_back(in_dg);
		}
	}
}