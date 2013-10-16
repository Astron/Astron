#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"
#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "core/logger.h"
#include "core/global.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <list>

static ConfigVariable<std::string> foldername("foldername", "objs");
LogCategory fsdb_log("fsdb", "Filesystem Database Engine");

class FSDBEngine : public IDatabaseEngine
{
	private:
		uint32_t m_next_id;
		std::list<uint32_t> m_free_ids;
		std::string m_foldername;

		// update_next_id updates "id.txt" on the disk with the next available id
		void update_next_id()
		{
			std::fstream file;
			file.open(m_foldername + "/id.txt", std::ios_base::out);
			if(file.is_open())
			{
				file << m_next_id;
				file.close();
			}
		}

		// update_free_ids updates "free.dat" on the disk with the current list of freed ids
		void update_free_ids()
		{
			std::fstream file;
			file.open(m_foldername + "/free.dat", std::ios_base::out | std::ios_base::binary);
			if(file.is_open())
			{
				Datagram dg;
				dg.add_uint32(m_free_ids.size());
				for(auto it = m_free_ids.begin(); it != m_free_ids.end(); ++it)
				{
					dg.add_uint32(*it);
				}
				file.write((char*)dg.get_data(), dg.size());
				file.close();
			}
		}

		// get_next_id returns the next available id to be used in object creation
		uint32_t get_next_id()
		{
			uint32_t do_id;
			if(m_next_id <= m_max_id)
			{
				do_id = m_next_id++;
				update_next_id();
			}
			else
			{
				// Dequeue id from list
				if(!m_free_ids.empty())
				{
					do_id = *m_free_ids.begin();
					m_free_ids.remove(do_id);
					update_free_ids();
				}
				else
				{
					return 0;
				}
			}
			return do_id;
		}
	public:
		FSDBEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
			IDatabaseEngine(dbeconfig, min_id, max_id),
			m_next_id(min_id), m_free_ids(),
			m_foldername(foldername.get_rval(m_config))
		{
			std::fstream file;

			// Get next id from "id.txt" in database
			file.open(m_foldername + "/id.txt", std::ios_base::in);
			if(file.is_open())
			{
				file >> m_next_id;
				file.close();
			}

			// Get list of free ids from "free.dat" in database
			file.open(m_foldername + "/free.dat", std::ios_base::in | std::ios_base::binary);
			if(file.is_open())
			{
				file.seekg(0, std::ios_base::end);
				uint32_t len = file.tellg();
				uint8_t *data = new uint8_t[len];
				file.seekg(0, std::ios_base::beg);
				file.read((char*)data, len);
				file.close();
				Datagram dg(data, len);
				delete [] data; //dg makes a copy
				DatagramIterator dgi(dg);

				uint32_t num_ids = dgi.read_uint32();
				for(uint32_t i = 0; i < num_ids; ++i)
				{
					auto k = dgi.read_uint32();
					fsdb_log.spam() << "Loaded free id: " << k << std::endl;
					m_free_ids.insert(m_free_ids.end(), k);
				}
			}
		}

		virtual uint32_t create_object(const DatabaseObject &dbo)
		{
			uint32_t do_id = get_next_id();
			if(do_id == 0)
			{
				return 0;
			}

			// Prepare object filename
			std::stringstream filename;
			filename << m_foldername << "/" << do_id << ".dat";

			// Write object to file
			std::fstream file;
			file.open(filename.str(), std::ios_base::out | std::ios_base::binary);
			if(file.is_open())
			{
				Datagram dg;
				dg.add_uint16(dbo.dc_id);
				dg.add_uint16(dbo.fields.size());
				for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
				{
					dg.add_uint16(it->first->get_number());
					dg.add_string(std::string(it->second.begin(), it->second.end()));
				}
				file.write((char*)dg.get_data(), dg.size());
				file.close();
				return do_id;
			}

			return 0;
		}

		virtual bool get_object(uint32_t do_id, DatabaseObject &dbo)
		{
			std::stringstream filename;
			filename << m_foldername << "/" << do_id << ".dat";

			std::fstream file;
			file.open(filename.str(), std::ios_base::in | std::ios_base::binary);
			if(file.is_open())
			{
				try
				{
					file.seekg(0, std::ios_base::end);
					uint32_t len = file.tellg();
					uint8_t *data = new uint8_t[len];
					file.seekg(0, std::ios_base::beg);
					file.read((char*)data, len);
					file.close();
					Datagram dg(data, len);
					delete [] data; //dg makes a copy
					DatagramIterator dgi(dg);

					dbo.dc_id = dgi.read_uint16();
					DCClass *dcc = g_dcf->get_class(dbo.dc_id);
					if(!dcc)
					{
						std::stringstream ss;
						ss << "DCClass " << dbo.dc_id << "does not exist.";
						throw std::runtime_error(ss.str());
					}

					uint16_t field_count = dgi.read_uint16();
					for(uint32_t i = 0; i < field_count; ++i)
					{
						uint16_t field_id = dgi.read_uint16();
						DCField *field = dcc->get_field_by_index(field_id);
						if(!field)
						{
							std::stringstream ss;
							ss << "DCField " << field_id << " does not exist in DCClass " << dbo.dc_id;
							throw std::runtime_error(ss.str());
						}
						std::string str = dgi.read_string();
						dbo.fields[field] = std::vector<uint8_t>(str.begin(), str.end());
					}
					return true;
				}
				catch(std::exception &e)
				{
					fsdb_log.error() << "Exception in get_object while trying to read do_id: #"
					                 << do_id << " e.what(): " << e.what() << std::endl;
				}
			}

			return false;
		}

		virtual void delete_object(uint32_t do_id)
		{
			std::stringstream filename;
			filename << foldername.get_rval(m_config) << "/" << do_id << ".dat";
			fsdb_log.debug() << "Deleting file: " << filename.str() << std::endl;
			if(!std::remove(filename.str().c_str()))
			{
				m_free_ids.insert(m_free_ids.end(), do_id);
				update_free_ids();
			}
		}

#define val_t std::vector<uint8_t>
#define map_t std::map< DCField*, std::vector<uint8_t> >
		DCClass* get_class(uint32_t do_id)
		{
			return NULL;
		}
		void del_field(uint32_t do_id, DCField* field)
		{
		}
		void del_fields(uint32_t do_id, const std::vector<DCField*> &fields)
		{
		}
		void set_field(uint32_t do_id, DCField* field, const val_t &value)
		{
		}
		void set_fields(uint32_t do_id, const map_t &fields)
		{
		}
		bool set_field_if_empty(uint32_t do_id, DCField* field, val_t &value)
		{
			return false;
		}
		bool set_fields_if_empty(uint32_t do_id, map_t &values)
		{
			return false;
		}
		bool set_field_if_equals(uint32_t do_id, DCField* field, const val_t &equal, val_t &value)
		{
			return false;
		}
		bool set_fields_if_equals(uint32_t do_id, const map_t &equals, map_t &values)
		{
			return false;
		}

		bool get_field(uint32_t do_id, const DCField* field, val_t &value)
		{
			return false;
		}
		bool get_fields(uint32_t do_id,  const std::vector<DCField*> &fields, map_t &values)
		{
			return false;
		}
#undef map_t
#undef val_t
};

DBEngineCreator<FSDBEngine> fsdbengine_creator("filesystem");