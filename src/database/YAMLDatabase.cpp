#include "OldDatabaseBackend.h"
#include "DBBackendFactory.h"
#include "DatabaseServer.h"

#include "core/global.h"
#include "util/DatagramIterator.h"
#include "dclass/value/format.h"
#include "dclass/value/parse.h"

#include <yaml-cpp/yaml.h>
#include <fstream> // std::ifstream
#include <list>    // std::list

using dclass::Class;
using dclass::Field;
using namespace std;

static ConfigGroup yaml_backend_config("yaml", db_backend_config);
static ConfigVariable<string> directory("directory", "yaml_db", yaml_backend_config);

class YAMLDatabase : public OldDatabaseBackend
{
  private:
    doid_t m_next_id;
    list<doid_t> m_free_ids;
    string m_directory;
    LogCategory *m_log;

    inline string filename(doid_t do_id)
    {
        stringstream filename;
        filename << m_directory << "/" << do_id << ".yaml";
        return filename.str();
    }

    inline bool load(doid_t do_id, YAML::Node &document)
    {
        ifstream stream(filename(do_id));
        document = YAML::Load(stream);
        if(!document.IsDefined() || document.IsNull()) {
            m_log->error() << "obj-" << do_id << " does not exist in database." << endl;
            return false;
        }
        if(!document["class"].IsDefined() || document["class"].IsNull()) {
            m_log->error() << filename(do_id) << " does not contain the 'class' key." << endl;
            return false;
        }
        if(!document["fields"].IsDefined() || document["fields"].IsNull()) {
            m_log->error() << filename(do_id) << " does not contain the 'fields' key." << endl;
            return false;
        }
        // Read object's DistributedClass
        string dc_name = document["class"].as<string>();
        if(!g_dcf->get_class_by_name(dc_name)) {
            m_log->error() << "Class '" << dc_name << "', loaded from '" << filename(do_id)
                           << "', does not exist." << endl;
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
        if(!m_free_ids.empty()) {
            out << YAML::Key << "free"
                << YAML::Value << YAML::BeginSeq;
            for(auto it = m_free_ids.begin(); it != m_free_ids.end(); ++it) {
                out << *it;
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;

        fstream file;
        file.open(m_directory + "/info.yaml", ios_base::out);
        if(file.is_open()) {
            file << out.c_str();
            file.close();
        }
    }

    // get_next_id returns the next available id to be used in object creation
    doid_t get_next_id()
    {
        doid_t do_id;
        if(m_next_id <= m_max_id) {
            do_id = m_next_id++;
        } else {
            // Dequeue id from list
            if(!m_free_ids.empty()) {
                do_id = *m_free_ids.begin();
                m_free_ids.remove(do_id);
            } else {
                return 0;
            }
        }

        update_info();
        return do_id;
    }

    vector<uint8_t> read_yaml_field(const Field* field, YAML::Node node, doid_t id)
    {
        bool error;
        string packed_data = dclass::parse_value(field->get_type(), node.as<string>(), error);
        if(error) {
            m_log->error() << "Failed parsing " << node.as<string>()
                           << " for field '" << field->get_name()
                           << "' of object " << id << " from database.\n";
            return vector<uint8_t>();
        }

        vector<uint8_t> result(packed_data.begin(), packed_data.end());
        return result;
    }

    void write_yaml_field(YAML::Emitter& out, const Field* field, const vector<uint8_t>& value)
    {
        out << YAML::Key << field->get_name() << YAML::Value;
        string packed_data(value.begin(), value.end());
        out << dclass::format_value(field->get_type(), packed_data);
    }

    bool write_yaml_object(doid_t do_id, const Class* dcc, const ObjectData &dbo)
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
        for(auto it = dbo.fields.begin(); it != dbo.fields.end(); ++it) {
            write_yaml_field(out, it->first, it->second);
        }
        out << YAML::EndMap
            << YAML::EndMap;

        // Print YAML to file
        fstream file;
        file.open(filename(do_id), ios_base::out);
        if(file.is_open()) {
            file << out.c_str();
            file.close();
            return true;
        }
        return false;
    }
  public:
    YAMLDatabase(ConfigNode dbeconfig, doid_t min_id, doid_t max_id) :
        OldDatabaseBackend(dbeconfig, min_id, max_id),
        m_next_id(min_id),
        m_free_ids(),
        m_directory(directory.get_rval(m_config))
    {
        stringstream log_name;
        log_name << "Database-YAML" << "(Range: [" << min_id << ", " << max_id << "])";
        m_log = new LogCategory("yamldb", log_name.str());

        // Open database info file
        ifstream infostream(m_directory + "/info.yaml");
        YAML::Node document = YAML::Load(infostream);

        if(document.IsDefined() && !document.IsNull()) {
            // Read next available id
            YAML::Node key_next = document["next"];
            if(key_next.IsDefined() && !key_next.IsNull()) {
                m_next_id = document["next"].as<doid_t>();
            }

            // Read available freed ids
            YAML::Node key_free = document["free"];
            if(key_free.IsDefined() && !key_free.IsNull()) {
                for(doid_t i = 0; i < key_free.size(); i++) {
                    m_free_ids.push_back(key_free[i].as<doid_t>());
                }
            }
        }

        // Close database info file
        infostream.close();
    }

    doid_t create_object(const ObjectData &dbo)
    {
        doid_t do_id = get_next_id();
        if(do_id == 0) {
            return 0;
        }

        const Class *dcc = g_dcf->get_class_by_id(dbo.dc_id);

        if(write_yaml_object(do_id, dcc, dbo)) {
            return do_id;
        }

        return 0;
    }

    void delete_object(doid_t do_id)
    {
        m_log->debug() << "Deleting file: " << filename(do_id) << endl;
        if(!remove(filename(do_id).c_str())) {
            m_free_ids.insert(m_free_ids.end(), do_id);
            update_info();
        }
    }

    bool get_object(doid_t do_id, ObjectData &dbo)
    {
        m_log->trace() << "Getting obj-" << do_id << " ..." << endl;

        // Open file for object
        YAML::Node document;
        if(!load(do_id, document)) {
            return false;
        }

        // Read object's DistributedClass
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        dbo.dc_id = dcc->get_id();

        // Read object's fields
        YAML::Node fields = document["fields"];
        for(auto it = fields.begin(); it != fields.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }

            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        return true;
    }

    const Class* get_class(doid_t do_id)
    {
        m_log->trace() << "Getting dclass of obj-" << do_id << endl;

        // Open file for object
        YAML::Node document;
        if(!load(do_id, document)) {
            return nullptr;
        }

        return g_dcf->get_class_by_name(document["class"].as<string>());
    }

    void del_field(doid_t do_id, const Field* field)
    {
        m_log->trace() << "Deleting field on obj-" << do_id << endl;

        // Read object from database
        YAML::Node document;
        if(!load(do_id, document)) {
            return;
        }

        // Get the fields from the file that are not being updated
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        // Remove field to be deleted from ObjectData
        dbo.fields.erase(field);

        // Write out new object to file
        write_yaml_object(do_id, dcc, dbo);
    }
    void del_fields(doid_t do_id, const FieldList &fields)
    {
        m_log->trace() << "Deleting fields on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            return;
        }

        // Get the fields from the file that are not being updated
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        for(auto it = fields.begin(); it != fields.end(); ++it) {
            dbo.fields.erase(*it);
        }
        write_yaml_object(do_id, dcc, dbo);
    }

    void set_field(doid_t do_id, const Field* field, const FieldValue &value)
    {
        m_log->trace() << "Setting field on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            return;
        }

        // Get the fields from the file that are not being updated
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        dbo.fields[field] = value;
        write_yaml_object(do_id, dcc, dbo);
    }

    void set_fields(doid_t do_id, const FieldValues &fields)
    {
        m_log->trace() << "Setting fields on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            return;
        }

        // Get the fields from the file that are not being updated
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }

            auto found = fields.find(field);
            if(found == fields.end()) {
                vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
                if(value.size() > 0) {
                    dbo.fields[field] = value;
                }
            }
        }

        // Add in the fields that are being updated:
        for(auto it = fields.begin(); it != fields.end(); ++it) {
            dbo.fields[it->first] = it->second;
        }

        write_yaml_object(do_id, dcc, dbo);
    }

    bool set_field_if_empty(doid_t do_id, const Field* field, FieldValue &value)
    {
        m_log->trace() << "Setting field if empty on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            value = vector<uint8_t>();
            return false;
        }

        // Get current field values from the file
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        auto found = dbo.fields.find(field);
        if(found != dbo.fields.end()) {
            value = found->second;
            return false;
        }

        dbo.fields[field] = value;
        write_yaml_object(do_id, dcc, dbo);
        return true;
    }

    bool set_field_if_equals(doid_t do_id, const Field* field,
                             const FieldValue &equal, FieldValue &value)
    {
        m_log->trace() << "Setting field if equal on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            value = vector<uint8_t>();
            return false;
        }

        // Get current field values from the file
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        auto found = dbo.fields.find(field);
        if(found == dbo.fields.end() || found->second != equal) {
            value = dbo.fields[field];
            return false;
        }

        dbo.fields[field] = value;
        write_yaml_object(do_id, dcc, dbo);
        return true;
    }
    bool set_fields_if_equals(doid_t do_id, const FieldValues &equals, FieldValues &values)
    {
        m_log->trace() << "Setting fields if equals on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            values.clear();
            return false;
        }

        // Get current field values from the file
        const Class* dcc = g_dcf->get_class_by_name(document["class"].as<string>());
        ObjectData dbo(dcc->get_id());
        YAML::Node existing = document["fields"];
        for(auto it = existing.begin(); it != existing.end(); ++it) {
            const Field* field = dcc->get_field_by_name(it->first.as<string>());
            if(!field) {
                m_log->warning() << "Field '" << it->first.as<string>()
                                 << "', loaded from '" << filename(do_id)
                                 << "', does not exist." << endl;
                continue;
            }
            vector<uint8_t> value = read_yaml_field(field, it->second, do_id);
            if(value.size() > 0) {
                dbo.fields[field] = value;
            }
        }

        // Check if equals matches current values
        bool fail = false;
        for(auto it = equals.begin(); it != equals.end(); ++it) {
            auto found = dbo.fields.find(it->first);
            if(found == dbo.fields.end()) {
                values.erase(it->first);
                fail = true;
            } else if(it->second != found->second) {
                values.erase(it->first);
                fail = true;
            }
        }

        // Return current values on failure
        if(fail) {
            for(auto it = values.begin(); it != values.end(); ++it) {
                it->second = dbo.fields[it->first];
            }
            return false;
        }

        // Update existing values on success
        for(auto it = values.begin(); it != values.end(); ++it) {
            dbo.fields[it->first] = it->second;
        }
        write_yaml_object(do_id, dcc, dbo);
        return true;
    }
    bool get_field(doid_t do_id, const Field* field, FieldValue &value)
    {
        m_log->trace() << "Getting field on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            return false;
        }

        // Get the fields from the file that are not being updated
        YAML::Node node = document["fields"][field->get_name()];
        if(!node.IsDefined() || node.IsNull()) {
            return false;
        }

        m_log->trace() << "Found requested field: " + field->get_name() << endl;

        value = read_yaml_field(field, node, do_id);
        if(value.size() > 0) {
            return true;
        }

        return false;
    }
    bool get_fields(doid_t do_id, const FieldList &fields, FieldValues &values)
    {
        m_log->trace() << "Getting fields on obj-" << do_id << endl;

        YAML::Node document;
        if(!load(do_id, document)) {
            return false;
        }

        // Get the fields from the file that are not being updated
        for(auto it = fields.begin(); it != fields.end(); ++it) {
            const Field* field = *it;
            m_log->trace() << "Searching for field: " << field->get_name() << endl;

            YAML::Node existing = document["fields"];
            for(auto it2 = existing.begin(); it2 != existing.end(); ++it2) {
                if(it2->first.as<string>() == field->get_name()) {
                    vector<uint8_t> value = read_yaml_field(field, it2->second, do_id);
                    if(value.size() > 0) {
                        values[*it] = value;
                        m_log->trace() << "Found requested field: " + field->get_name() << endl;
                    }
                }
            }
        }
        return true;
    }
};

DBBackendFactoryItem<YAMLDatabase> yamldb_factory("yaml");
