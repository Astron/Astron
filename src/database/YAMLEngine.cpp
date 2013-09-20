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


#define AS_BYTES(value, len) std::vector<uint8_t>((uint8_t*)&value, (uint8_t*)&value + len)


static ConfigVariable<std::string> foldername("foldername", "yaml_db");
LogCategory yamldb_log("yamldb", "YAML Database Engine");

class YAMLEngine : public IDatabaseEngine
{
private:
	uint32_t m_next_id;
	std::list<uint32_t> m_free_ids;
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
	uint32_t get_next_id()
	{
		uint32_t do_id;
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

	std::vector<uint8_t> read_yaml_field(DCField* field, YAML::Node node)
	{
		DCAtomicField* atomic = field->as_atomic_field();
		if(atomic && atomic->get_num_elements() == 1)
		{
			switch(atomic->get_element_type(0))
			{
				case ST_char:
				{
					char value = node.as<char>();
					return AS_BYTES(value, 1);
				}
				break;
				case ST_int8:
				{
					char value = node.as<char>();
					return AS_BYTES(value, 1);
				}
				break;
				case ST_int16:
				{
					short value = node.as<short>();
					return AS_BYTES(value, 2);
				}
				break;
				case ST_int32:
				{
					int value = node.as<int>();
					return AS_BYTES(value, 4);
				}
				break;
				case ST_int64:
				{
					long long value = node.as<long long>();
					return AS_BYTES(value, 8);
				}
				break;
				case ST_uint8:
				{
					uint8_t value = node.as<uint8_t>();
					return AS_BYTES(value, 1);
				}
				break;
				case ST_uint16:
				{
					uint16_t value = node.as<uint16_t>();
					return AS_BYTES(value, 2);
				}
				break;
				case ST_uint32:
				{
					uint32_t value = node.as<uint32_t>();
					return AS_BYTES(value, 4);
				}
				break;
				case ST_uint64:
				{
					uint64_t value = node.as<uint64_t>();
					return AS_BYTES(value, 8);
				}
				break;
				case ST_float64:
				{
					double value = node.as<double>();
					return AS_BYTES(value, 8);
				}
				break;
				case ST_string:
				{
					std::string value = node.as<std::string>();
					uint16_t length = value.length();

					std::vector<uint8_t> str = AS_BYTES(length, 2);
					str.insert(str.end(), value.begin(), value.end());
					return str;
				}
				break;
				default:
					std::string value = node.as<std::string>();
					return std::vector<uint8_t>(value.begin(), value.end());
			}

		}

		return std::vector<uint8_t>();
	}

	void write_yaml_field(YAML::Emitter& out, DCField* field, const std::vector<uint8_t>& value)
	{
		out << YAML::Key << field->get_name();
		DCAtomicField* atomic = field->as_atomic_field();

		if(atomic && atomic->get_num_elements() == 1)
		{
			out << YAML::Value;
			switch(atomic->get_element_type(0))
			{
				case ST_char:
				{
					out << *(char*)(&value[0]);
				}
				break;
				case ST_int8:
				{
					// Cast as short, because char treated as character data
					out << *(short*)(&value[0]);
				}
				break;
				case ST_int16:
				{
					out << *(short*)(&value[0]);
				}
				break;
				case ST_int32:
				{
					out << *(int*)(&value[0]);
				}
				break;
				case ST_int64:
				{
					out << *(long long*)(&value[0]);
				}
				break;
				case ST_uint8:
				{
					out << *(uint8_t*)(&value[0]);
				}
				break;
				case ST_uint16:
				{
					out << *(uint16_t*)(&value[0]);
				}
				break;
				case ST_uint32:
				{
					out << *(uint32_t*)(&value[0]);
				}
				break;
				case ST_uint64:
				{
					out << *(uint64_t*)(&value[0]);
				}
				break;
				case ST_float64:
				{
					out << *(double*)(&value[0]);
				}
				break;
				case ST_string:
				{
					// Skip the length AS_BYTES
					out << std::string(value.begin()+2, value.end());
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
	YAMLEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
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
				m_next_id = document["next"].as<uint32_t>();
			}

			// Read available freed ids
			YAML::Node key_free = document["free"];
			if(!key_free.IsNull())
			{
				for(uint32_t i = 0; i < key_free.size(); i++)
				{
					m_free_ids.push_back(key_free[i].as<uint32_t>());
				}
			}
		}

		// Close database info file
		infostream.close();
	}

	uint32_t create_object(const DatabaseObject &dbo)
	{
		uint32_t do_id = get_next_id();
		if(do_id == 0)
		{
			return 0;
		}

		DCClass *dcc = g_dcf->get_class(dbo.dc_id);

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
			write_yaml_field(out, it->first, it->second);
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

	bool get_object(uint32_t do_id, DatabaseObject &dbo)
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
		DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
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

			dbo.fields[field] = read_yaml_field(field, it->second);
		}

		return true;
	}

	void delete_object(uint32_t do_id)
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