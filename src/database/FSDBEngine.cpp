#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"
#include "util/Datagram.h"
#include <fstream>
#include <sstream>

ConfigVariable<std::string> folder_name("foldername", "objs");

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

class FSDBEngine : public IDatabaseEngine
{
	private:
		unsigned int m_next_id;
	public:
		FSDBEngine(DBEngineConfig dbeconfig, unsigned int start_id) : IDatabaseEngine(dbeconfig, start_id),
			m_next_id(start_id)
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
		}

		virtual unsigned int get_next_id()
		{
			return m_next_id;
		}

		virtual bool create_object(unsigned int do_id, const std::map<DCField*, std::string> &fields)
		{
			if(do_id != m_next_id)
			{
				return false;
			}
			std::stringstream ss;
			ss << folder_name.get_rval(m_dbeconfig) << "/id.txt";
			WriteIDFile(ss.str(), ++m_next_id);

			ss.str("");
			ss << folder_name.get_rval(m_dbeconfig) << "/" << do_id << ".dat";
			std::fstream file;
			file.open(ss.str(), std::ios_base::out | std::ios_base::binary);
			if(file.is_open())
			{
				Datagram dg;
				dg.add_uint16(fields.size());
				for(auto it = fields.begin(); it != fields.end(); ++it)
				{
					dg.add_uint16(it->first->get_number());
					dg.add_string(it->second);
				}
				file.write(dg.get_data(), dg.get_buf_end());
				file.close();
				return true;
			}
			
			return false;//TODO: implement me so I can return true;
		}
};

DBEngineCreator<FSDBEngine> fsdbengine_creator("filesystem");