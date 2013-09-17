#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "core/logger.h"
#include "core/global.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <list>

ConfigVariable<std::string> foldername("foldername", "yaml_db");
LogCategory fsdb_log("YAML-DB", "YAML Database Engine");

class YAMLEngine : public IDatabaseEngine
{
private:
	unsigned int m_next_id;
	std::list<unsigned int> m_free_ids;
	std::string m_foldername;

	// update_next_id writes replaces the "next" field of info.yaml
	// with the current m_next_id value;
	void update_next_id()
	{

	}

	// update_free_ids replaces the "free" list of ids in info.yaml
	// with the current list of free ids.
	void update_free_ids()
	{

	}
public:
	YAMLEngine(DBEngineConfig dbeconfig, unsigned int start_id) :
		IDatabaseEngine(dbeconfig, start_id),
		m_next_id(start_id),
		m_free_ids()
		m_foldername(foldername.get_rval(m_dbeconfig))
	{
		// Open database info file
		std::ifstream info(m_foldername + "/info.yaml");
		YAML::Parser parser(info);
		YAML::Node document;

		// Read existing database info if file exists
		if(parser.GetNextDocument(document))
		{
			// Read next available id
			if(auto next = document.FindValue("next"))
			{
				next >> m_next_id;
			}

			// Read available freed ids
			if(auto freed = document.FindValue("free"))
			{
				for(unsigned int i = 0; i < freed.size(); i++)
				{
					unsigned int id;
					freed[i] >> id;
					m_free_ids.push_back(id);
				}
			}
		}

		// Close database info file
		info.close();
	};

	unsigned int get_next_id()
	{
		// Get first free id from queue
		if(!m_free_ids.empty())
		{
			return *m_free_ids.begin();
		}
		return m_next_id;
	}

	bool create_object(const DatabaseObject &dbo)
	{
		if(dbo.do_id != get_next_id())
		{
			return false;
		}
		if(!m_free_ids.empty())
		{
			m_free_ids.remove(dbo.do_id);
			update_free_ids();
		}
		++m_next_id;
		update_next_id();
	}

	bool get_object(DatabaseObject &dbo)
	{
	}

	void delete_object(unsigned int do_id)
	{

	}

};

DBEngineCreator<FSDBEngine> yamlengine_creator("yaml");