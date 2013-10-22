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

		inline std::string filename(uint32_t do_id)
		{
			std::stringstream filename;
			filename << m_foldername << "/" << do_id << ".yaml";
			return filename.str();
		}

		inline bool load(uint32_t do_id, YAML::Node &document)
		{
			std::ifstream stream(filename(do_id));
			document = YAML::Load(stream);
			if(!document.IsDefined() || document.IsNull())
			{
				yamldb_log.error() << "obj-" << do_id << " does not exist in database." << std::endl;
				return false;
			}
			if(!document["class"].IsDefined() || document["class"].IsNull())
			{
				yamldb_log.error() << filename(do_id) << " does not contain the 'class' key." << std::endl;
				return false;
			}
			if(!document["fields"].IsDefined() || document["fields"].IsNull())
			{
				yamldb_log.error() << filename(do_id) << " does not contain the 'fields' key." << std::endl;
				return false;
			}
			// Read object's DistributedClass
			std::string dc_name = document["class"].as<std::string>();
			if(!g_dcf->get_class_by_name(dc_name))
			{
				yamldb_log.error() << "Class '" << dc_name << "', loaded from '" << filename(do_id)
				                   << "', does not exist." << std::endl;
				return false;
			}

			return true;
		}

		// update_info writes m_next_id and m_free_ids to "info.yaml"
		void update_info()
		{
			YAML::Emitter out;
			out << YAML::BeginMap
			    << YAML::Key << "next"
			    << YAML::Value << m_next_id;
			if(!m_free_ids.empty())
			{
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

		std::vector<uint8_t> read_yaml_field(const DCField* field, YAML::Node node)
		{
			std::string packed_data = const_cast<DCField*>(field)->parse_string(node.as<std::string>());
			std::vector<uint8_t> result(packed_data.begin(), packed_data.end());
			return result;
		}

		void write_yaml_field(YAML::Emitter& out, const DCField* field, const std::vector<uint8_t>& value)
		{
			out << YAML::Key << field->get_name() << YAML::Value;
			std::string packed_data(value.begin(), value.end());
			out << const_cast<DCField*>(field)->format_data(packed_data, false);
		}

		bool write_yaml_object(uint32_t do_id, DCClass* dcc, const DatabaseObject &dbo)
		{
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

			// Print YAML to file
			std::fstream file;
			file.open(filename(do_id), std::ios_base::out);
			if(file.is_open())
			{
				file << out.c_str();
				file.close();
				return true;
			}
			return false;
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

			if(document.IsDefined() && !document.IsNull())
			{
				// Read next available id
				YAML::Node key_next = document["next"];
				if(key_next.IsDefined() && !key_next.IsNull())
				{
					m_next_id = document["next"].as<uint32_t>();
				}

				// Read available freed ids
				YAML::Node key_free = document["free"];
				if(key_free.IsDefined() && !key_free.IsNull())
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

			if(write_yaml_object(do_id, dcc, dbo))
			{
				return do_id;
			}

			return 0;
		}

		void delete_object(uint32_t do_id)
		{
			yamldb_log.debug() << "Deleting file: " << filename(do_id) << std::endl;
			if(!std::remove(filename(do_id).c_str()))
			{
				m_free_ids.insert(m_free_ids.end(), do_id);
				update_info();
			}
		}

		bool get_object(uint32_t do_id, DatabaseObject &dbo)
		{
			yamldb_log.spam() << "Getting obj-" << do_id << " ..." << std::endl;

			// Open file for object
			YAML::Node document;
			if(!load(do_id, document))
			{
				return false;
			}

			// Read object's DistributedClass
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			dbo.dc_id = dcc->get_number();

			// Read object's fields
			YAML::Node fields = document["fields"];
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}

				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			return true;
		}

		DCClass* get_class(uint32_t do_id)
		{
			yamldb_log.spam() << "Getting dclass of obj-" << do_id << std::endl;

			// Open file for object
			YAML::Node document;
			if(!load(do_id, document))
			{
				return NULL;
			}

			return g_dcf->get_class_by_name(document["class"].as<std::string>());
		}


#define val_t std::vector<uint8_t>
#define map_t std::map<DCField*, std::vector<uint8_t> >
		void del_field(uint32_t do_id, DCField* field)
		{
			yamldb_log.spam() << "Deleting field on obj-" << do_id << std::endl;

			// Read object from database
			YAML::Node document;
			if(!load(do_id, document))
			{
				return;
			}

			// Get the fields from the file that are not being updated
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			// Remove field to be deleted from DatabaseObject
			dbo.fields.erase(field);

			// Write out new object to file
			write_yaml_object(do_id, dcc, dbo);
		}
		void del_fields(uint32_t do_id, const std::vector<DCField*> &fields)
		{
			yamldb_log.spam() << "Deleting fields on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				return;
			}

			// Get the fields from the file that are not being updated
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				dbo.fields.erase(*it);
			}
			write_yaml_object(do_id, dcc, dbo);
		}
		void set_field(uint32_t do_id, DCField* field, const val_t &value)
		{
			yamldb_log.spam() << "Setting field on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				return;
			}

			// Get the fields from the file that are not being updated
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			dbo.fields[field] = value;
			write_yaml_object(do_id, dcc, dbo);
		}

		void set_fields(uint32_t do_id, const map_t &fields)
		{
			yamldb_log.spam() << "Setting fields on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				return;
			}

			// Get the fields from the file that are not being updated
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}

				auto found = fields.find(field);
				if(found == fields.end())
				{
					std::vector<uint8_t> value = read_yaml_field(field, it->second);
					if(value.size() > 0)
					{
						dbo.fields[field] = value;
					}
					dbo.fields[field] = read_yaml_field(field, it->second);
				}
				else
				{
					dbo.fields[field] = found->second;
				}
			}

			write_yaml_object(do_id, dcc, dbo);
		}
		bool set_field_if_empty(uint32_t do_id, DCField* field, val_t &value)
		{
			yamldb_log.spam() << "Setting field if empty on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				value = std::vector<uint8_t>();
				return false;
			}

			// Get current field values from the file
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			auto found = dbo.fields.find(field);
			if(found != dbo.fields.end())
			{
				value = found->second;
				return false;
			}

			dbo.fields[field] = value;
			write_yaml_object(do_id, dcc, dbo);
			return true;
		}
		bool set_field_if_equals(uint32_t do_id, DCField* field, const val_t &equal, val_t &value)
		{
			yamldb_log.spam() << "Setting field if equal on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				value = std::vector<uint8_t>();
				return false;
			}

			// Get current field values from the file
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			auto found = dbo.fields.find(field);
			if(found == dbo.fields.end() || found->second != equal)
			{
				value = dbo.fields[field];
				return false;
			}

			dbo.fields[field] = value;
			write_yaml_object(do_id, dcc, dbo);
			return true;
		}
		bool set_fields_if_equals(uint32_t do_id, const map_t &equals, map_t &values)
		{
			yamldb_log.spam() << "Setting fields if equals on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				values.clear();
				return false;
			}

			// Get current field values from the file
			DCClass* dcc = g_dcf->get_class_by_name(document["class"].as<std::string>());
			DatabaseObject dbo(dcc->get_number());
			YAML::Node existing = document["fields"];
			for(auto it = existing.begin(); it != existing.end(); ++it)
			{
				DCField* field = dcc->get_field_by_name(it->first.as<std::string>());
				if(!field)
				{
					yamldb_log.warning() << "Field '" << it->first.as<std::string>()
					                     << "', loaded from '" << filename(do_id)
					                     << "', does not exist." << std::endl;
					continue;
				}
				std::vector<uint8_t> value = read_yaml_field(field, it->second);
				if(value.size() > 0)
				{
					dbo.fields[field] = value;
				}
			}

			// Check if equals matches current values
			bool fail = false;
			for(auto it = equals.begin(); it != equals.end(); ++it)
			{
				auto found = dbo.fields.find(it->first);
				if(found == dbo.fields.end())
				{
					values.erase(it->first);
					fail = true;
				}
				else if(it->second != found->second)
				{
					values.erase(it->first);
					fail = true;
				}
			}

			// Return current values on failure
			if(fail)
			{
				for(auto it = values.begin(); it != values.end(); ++it)
				{
					it->second = dbo.fields[it->first];
				}
				return false;
			}

			// Update existing values on success
			for(auto it = values.begin(); it != values.end(); ++it)
			{
				dbo.fields[it->first] = it->second;
			}
			write_yaml_object(do_id, dcc, dbo);
			return true;
		}
		bool get_field(uint32_t do_id, const DCField* field, val_t &value)
		{
			yamldb_log.spam() << "Getting field on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				return false;
			}

			// Get the fields from the file that are not being updated
			YAML::Node node = document["fields"][field->get_name()];
			if(!node.IsDefined() || node.IsNull())
			{
				return false;
			}

			yamldb_log.spam() << "Found requested field: " + field->get_name() << std::endl;

			value = read_yaml_field(field, node);
			if(value.size() > 0)
			{
				return true;
			}

			return false;
		}
		bool get_fields(uint32_t do_id, const std::vector<DCField*> &fields, map_t &values)
		{
			yamldb_log.spam() << "Getting fields on obj-" << do_id << std::endl;

			YAML::Node document;
			if(!load(do_id, document))
			{
				return false;
			}

			// Get the fields from the file that are not being updated
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				DCField* field = *it;
				yamldb_log.spam() << "Searching for field: " << field->get_name() << std::endl;
				YAML::Node existing = document["fields"];
				for(auto it2 = existing.begin(); it2 != existing.end(); ++it2)
				{
					if(it2->first.as<std::string>() == field->get_name())
					{
						std::vector<uint8_t> value = read_yaml_field(field, it2->second);
						if(value.size() > 0)
						{
							values[*it] = value;
							yamldb_log.spam() << "Found requested field: " + field->get_name() << std::endl;
						}
					}
				}
			}
			return true;
		}
#undef map_t
#undef val_t
};

DBEngineCreator<YAMLEngine> yamlengine_creator("yaml");
