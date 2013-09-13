#include "Database.h"
#include "DatabaseFactory.h"
#include "core/global.h"
#include <yaml-cpp/yaml.h>

ConfigVariable<std::string> storage_file("file", "database.yaml");

class YAMLDatabase : public Database
{
public:
	YAMLDatabase(DatabaseConfig dbconfig) : Database(dbconfig) {
		filename = storage_file.get_rval(dbconfig);
		file.open(filename);
	}

	~YAMLDatabase() {}

	void create_object(unsigned int do_id,
					   unsigned short dc_id,
					   unsigned short field_count,
					   DatagramIterator& data)
	{
		DCClass *dcc = gDCF->get_class(dc_id);

		YAML::Emitter out;
		out << YAML::BeginSeq;
			out << YAML::BeginMap;
			out << YAML::Key << do_id;
			out << YAML::Value << YAML::BeginMap;
				out << YAML::Key << "class";
				out << YAML::Value << dcc->get_name();
				out << YAML::Key << "fields";
				out << YAML::Value << YAML::BeginMap;
				for(int i=0; i < field_count; ++i)
				{
					std::string value;
					DCField *field = dcc->get_field_by_index(data.read_uint16());
					data.unpack_field(field, value);
					out << YAML::Key << field->get_name();
					out << YAML::Value << value;
				}
				out << YAML::EndMap;
			out << YAML::Value << YAML::EndMap;
		out << YAML::EndSeq;

		file << out.c_str();
	}
	void delete_object(unsigned int do_id) {}
	void select_object(unsigned int do_id /* TODO: Return values */) {}
	void update_object(unsigned int do_id,
							   unsigned short field_count,
							   DatagramIterator& data) {}
	void update_object_if_equals(unsigned int do_id,
										 unsigned short field_count,
										 DatagramIterator& data) {}

	void select_query(/* TODO: Args */) {}
	void update_query(/* TODO: Args */) {}
	void delete_query(/* TODO: Args */) {}
private:
	std::ofstream file;
	std::string filename;
};

DatabaseItem<YAMLDatabase> yaml_fact("yaml");