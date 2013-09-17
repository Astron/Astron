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

ConfigVariable<std::string> folder_name("foldername", "objs");
LogCategory fsdb_log("fsdb", "Filesystem Database Engine");

void WriteIDFile(const std::string &filename, unsigned int id)
{
	std::fstream file;
	file.open(filename, std::ios_base::out);
	if(file.is_open())
	{
		file << id;
		file.close();
	}
}

void WriteFreeList(const std::string &filename, std::list<unsigned int> &free_list)
{
	std::fstream file;
	file.open(filename, std::ios_base::out | std::ios_base::binary);
	if(file.is_open())
	{
		Datagram dg;
		dg.add_uint32(free_list.size());
		for(auto it = free_list.begin(); it != free_list.end(); ++it)
		{
			dg.add_uint32(*it);
		}
		file.write(dg.get_data(), dg.get_buf_end());
		file.close();
	}
}

class FSDBEngine : public IDatabaseEngine
{
private:
	unsigned int m_next_id;
	std::list<unsigned int> m_free_ids;
public:
	FSDBEngine(DBEngineConfig dbeconfig, unsigned int start_id) : IDatabaseEngine(dbeconfig, start_id),
		m_next_id(start_id), m_free_ids()
	{
		std::fstream file;
		std::stringstream ss;
		ss << folder_name.get_rval(m_dbeconfig) << "/id.txt";
		file.open(ss.str(), std::ios_base::in);
		if(file.is_open())
		{
			file >> m_next_id;
			file.close();
		}

		ss.str("");
		ss << folder_name.get_rval(m_dbeconfig) << "/free.dat";
		file.open(ss.str(), std::ios_base::in | std::ios_base::binary);
		if(file.is_open())
		{
			file.seekg(0, std::ios_base::end);
			unsigned int len = file.tellg();
			char *data = new char[len];
			file.seekg(0, std::ios_base::beg);
			file.read(data, len);
			file.close();
			Datagram dg(std::string(data, len));
			delete [] data; //dg makes a copy
			DatagramIterator dgi(dg);

			unsigned int num_ids = dgi.read_uint32();
			for(unsigned int i = 0; i < num_ids; ++i)
			{
				m_free_ids.insert(m_free_ids.end(), dgi.read_uint32());
			}
		}
	}

	virtual unsigned int get_next_id()
	{
		if(!m_free_ids.empty())
		{
			return *m_free_ids.begin();
		}
		return m_next_id;
	}

	virtual bool create_object(const DatabaseObject &dbo)
	{
		if(dbo.do_id != get_next_id())
		{
			return false;
		}
		if(!m_free_ids.empty())
		{
			m_free_ids.remove(dbo.do_id);
			std::stringstream ss;
			ss << folder_name.get_rval(m_dbeconfig) << "/free.dat";
			WriteFreeList(ss.str(), m_free_ids);
		}
		std::stringstream ss;
		ss << folder_name.get_rval(m_dbeconfig) << "/id.txt";
		WriteIDFile(ss.str(), ++m_next_id);

		ss.str("");
		ss << folder_name.get_rval(m_dbeconfig) << "/" << dbo.do_id << ".dat";
		std::fstream file;
		file.open(ss.str(), std::ios_base::out | std::ios_base::binary);
		if(file.is_open())
		{
			Datagram dg;
			dg.add_uint16(dbo.dc_id);
			dg.add_uint16(dbo.fields.size());
			for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
			{
				dg.add_uint16(it->first->get_number());
				dg.add_string(it->second);
			}
			file.write(dg.get_data(), dg.get_buf_end());
			file.close();
			return true;
		}
		
		return false;
	}

	virtual bool get_object(DatabaseObject &dbo)
	{
		std::stringstream ss;
		ss << folder_name.get_rval(m_dbeconfig) << "/" << dbo.do_id << ".dat";
		std::fstream file;
		file.open(ss.str(), std::ios_base::in | std::ios_base::binary);
		if(file.is_open())
		{
			try
			{
				file.seekg(0, std::ios_base::end);
				unsigned int len = file.tellg();
				char *data = new char[len];
				file.seekg(0, std::ios_base::beg);
				file.read(data, len);
				file.close();
				Datagram dg(std::string(data, len));
				delete [] data; //dg makes a copy
				DatagramIterator dgi(dg);

				dbo.dc_id = dgi.read_uint16();
				DCClass *dcc = gDCF->get_class(dbo.dc_id);
				if(!dcc)
				{
					std::stringstream ss;
					ss << "DCClass " << dbo.dc_id << "does not exist.";
					throw std::runtime_error(ss.str());
				}

				unsigned short field_count = dgi.read_uint16();
				for(unsigned int i = 0; i < field_count; ++i)
				{
					unsigned short field_id = dgi.read_uint16();
					DCField *field = dcc->get_field_by_index(field_id);
					if(!field)
					{
						std::stringstream ss;
						ss << "DCField " << field_id << " does not exist in DCClass " << dbo.dc_id;
						throw std::runtime_error(ss.str());
					}
					dbo.fields[field] = dgi.read_string();
				}
				return true;
			}
			catch (std::exception &e)
			{
				fsdb_log.error() << "Exception in get_object while trying to read doId: "
					<< dbo.do_id << " e.what(): " << e.what() << std::endl;
			}
		}

		return false;
	}

	virtual void delete_object(unsigned int do_id)
	{
		std::stringstream ss;
		ss << folder_name.get_rval(m_dbeconfig) << "/" << do_id << ".dat";
		fsdb_log.debug() << "Deleting file: " << ss.str() << std::endl;
		if(!std::remove(ss.str().c_str()))
		{
			m_free_ids.insert(m_free_ids.end(), do_id);
			std::stringstream ss;
			ss << folder_name.get_rval(m_dbeconfig) << "/free.dat";
			WriteFreeList(ss.str(), m_free_ids);
		}
	}
};

DBEngineCreator<FSDBEngine> fsdbengine_creator("filesystem");