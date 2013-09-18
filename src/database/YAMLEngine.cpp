#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "core/logger.h"
#include "core/global.h"
#include "dcparser/dcSubatomicType.h"
#include "dcparser/dcAtomicField.h"

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
		out << YAML::BeginMap
			<< YAML::Key << "next"
			<< YAML::Value << m_next_id;
		if(!m_free_ids.empty()) {
			out << YAML::Key << "free"
				<< YAML::Value << YAML::BeginSeq;
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

	inline void output_field(YAML::Emitter& out, DCField* field, std::string value)
	{
		out << YAML::Key << field->get_name();
		DCAtomicField* atomic = field->as_atomic_field();

		if(atomic && atomic->get_num_elements() == 1)
		{
			out << YAML::Value;
			switch(atomic->get_element_type(0)) {
				case ST_char:
				{
					out << *(char*)(value.c_str());
				}
				break;
				case ST_int8:
				{
					// Cast as short, because char treated as character data
					out << *(short*)(value.c_str());
				}
				break;
				case ST_int16:
				{
					out << *(short*)(value.c_str());
				}
				break;
				case ST_int32:
				{
					out << *(int*)(value.c_str());
				}
				break;
				case ST_int64:
				{
					out << *(long long*)(value.c_str());
				}
				break;
				case ST_uint8:
				{
					out << *(unsigned char*)(value.c_str());
				}
				break;
				case ST_uint16:
				{
					out << *(unsigned short*)(value.c_str());
				}
				break;
				case ST_uint32:
				{
					out << *(unsigned int*)(value.c_str());
				}
				break;
				case ST_uint64:
				{
					out << *(unsigned long long*)(value.c_str());
				}
				break;
				case ST_float64:
				{
					out << *(double*)(value.c_str());
				}
				break;
				case ST_string:
				{
					// Skip the length bytes
					out << value.substr(2);
				}
				break;
				default:
					out << value;
			}
			return;
		}

		yamldb_log.error() << "Recieved non-atomic field: " << field->get_name() << std::endl;
		out << YAML::Value << value;
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
	}

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
		out << YAML::BeginMap
			<< YAML::Key << "id"
			<< YAML::Value << do_id
			<< YAML::Key << "class"
			<< YAML::Value << dcc->get_name()
			<< YAML::Key << "fields"
			<< YAML::Value << YAML::BeginMap;
		for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it)
		{
			output_field(out, it->first, it->second);
		}
		out << YAML::EndMap
			<< YAML::EndMap;

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
		yamldb_log.spam() << "Getting object #" << do_id << " ..." << std::endl;

		// Prepare object filename
		std::stringstream filename;
		filename << m_foldername << "/" << do_id << ".yaml";

		// Open database object file
		std::ifstream objstream(filename.str());
		YAML::Node document = YAML::Load(objstream);

		if(document.IsNull()) {
			yamldb_log.error() << "Object #" << do_id << " does not exist in database." << std::endl;
			return false;
		}

		// Read object's DistributedClass
		YAML::Node key_class = document["class"];
		if(key_class.IsNull())
		{
			yamldb_log.error() << filename.str() << " does not contain the 'class' key." << std::endl;
			return false;
		}
		DCClass* dcc = gDCF->get_class_by_name(document["class"].as<std::string>());
		if(!dcc) {
			yamldb_log.error() << "Class '" << document["class"].as<std::string>() << "', loaded from '" << filename.str() << "', does not exist." << std::endl;
			return false;
		}
		dbo.dc_id = dcc->get_number();

		// Read object's fields
		YAML::Node key_fields = document["fields"];
		if(key_fields.IsNull())
		{
			yamldb_log.error() << filename.str() << " does not contain the 'fields' key." << std::endl;
			return false;
		}
		for(auto it = key_fields.begin(); it != key_fields.end(); ++it)
		{
			DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
			if(!field) {
				yamldb_log.warning() << "Field '" << it->first.as<std::string>() << "', loaded from '" << filename.str() << "', does not exist." << std::endl;
				continue;
			}

			DCAtomicField* atomic = field->as_atomic_field();
			if(atomic && atomic->get_num_elements() == 1 && atomic->get_element_type(0) == ST_string)
			{
				std::string str = it->second.as<std::string>();
				unsigned int len = str.length();
				char lenstr[2];

				memcpy(lenstr, (char*)&len, 2);
				dbo.fields[field] = std::string(lenstr) + str;
			}
			else
			{
				dbo.fields[field] = it->second.as<std::string>();
			}
		}

		return true;
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