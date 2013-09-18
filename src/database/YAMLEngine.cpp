#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "core/logger.h"
#include "core/global.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <list>

static ConfigVariable<std::string> foldername("foldername", "yaml_db");
LogCategory yamldb_log("yamldb", "YAML Database Engine");

class YAMLEngine : public IDatabaseEngine
{
private:
	unsigned int m_next_id;
	std::list<unsigned int> m_free_ids;
	std::string m_foldername;

	// update_info writes m_next_id and m_free_ids to "info.yaml"
	void update_info()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "next";
		out << YAML::Value << m_next_id;
		if(!m_free_ids.empty()) {
			out << YAML::Key << "free";
			out << YAML::Value << YAML::BeginSeq;
			for(auto it = m_free_ids.begin(); it != m_free_ids.end(); ++it)
			{
				out << *it;
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

		std::fstream file;
		file.open(m_foldername + "/info.yaml", std::ios_base::out);
		if(file.is_open())
		{
			file << out.c_str();
			file.close();
		}
	}

	// get_next_id returns the next available id to be used in object creation
	unsigned int get_next_id()
	{
		unsigned int do_id;
		if(m_next_id <= m_max_id)
		{
			do_id = m_next_id++;
		}
		else
		{
			// Dequeue id from list
			if(!m_free_ids.empty())
			{
				do_id = *m_free_ids.begin();
				m_free_ids.remove(do_id);
			}
			else
			{
				return 0;
			}
		}

		update_info();
		return do_id;
	}
public:
	YAMLEngine(DBEngineConfig dbeconfig, unsigned int min_id, unsigned int max_id) :
		IDatabaseEngine(dbeconfig, min_id, max_id),
		m_next_id(min_id),
		m_free_ids(),
		m_foldername(foldername.get_rval(m_config))
	{
		// Open database info file
		std::ifstream infostream(m_foldername + "/info.yaml");
		YAML::Node document = YAML::Load(infostream);

		if(!document.IsNull()) {
			// Read next available id
			YAML::Node key_next = document["next"];
			if(!key_next.IsNull())
			{
				m_next_id = document["next"].as<unsigned int>();
			}

			// Read available freed ids
			YAML::Node key_free = document["free"];
			if(!key_free.IsNull())
			{
				for(unsigned int i = 0; i < key_free.size(); i++)
				{
					m_free_ids.push_back(key_free[i].as<unsigned int>());
				}
			}
		}

		// Close database info file
		infostream.close();
	};

	unsigned int create_object(const DatabaseObject &dbo)
	{
		unsigned int do_id = get_next_id();
		if(do_id == 0)
		{
			return 0;
		}

		DCClass *dcc = gDCF->get_class(dbo.dc_id);

		// Build object as YAMl output
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "id";
		out << YAML::Value << do_id;
		out << YAML::Key << "class";
		out << YAML::Value << dcc->get_name();

		// Prepare object filename
		std::stringstream filename;
		filename << m_foldername << "/" << do_id << ".yaml";

		// Print YAML to file
		std::fstream file;
		file.open(filename.str(), std::ios_base::out);
		if(file.is_open())
		{
			file << out.c_str();
			file.close();
			return do_id;
		}		

		return 0;
	}

	bool get_object(unsigned int do_id, DatabaseObject &dbo)
	{
		return false;
	}

	void delete_object(unsigned int do_id)
	{
		std::stringstream filename;
		filename << m_foldername << "/" << do_id << ".yaml";
		yamldb_log.debug() << "Deleting file: " << filename.str() << std::endl;
		if(!std::remove(filename.str().c_str()))
		{
			m_free_ids.insert(m_free_ids.end(), do_id);
			update_info();
		}
	}

};

DBEngineCreator<YAMLEngine> yamlengine_creator("yaml");