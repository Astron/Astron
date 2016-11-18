#include "ConfigGroup.h"
#include "core/Logger.h"
using namespace std;

LogCategory config_log("config", "Config");

void config_error(const std::string& msg)
{
    config_log.error() << msg << "\n";
}

ConfigGroup& ConfigGroup::root()
{
    static ConfigGroup* root = new ConfigGroup();
    return *root;
}

// root constructor
ConfigGroup::ConfigGroup() : m_name(""), m_parent(nullptr)
{
}

// constructor
ConfigGroup::ConfigGroup(const string& name, ConfigGroup& parent) : m_name(name), m_parent(&parent)
{
    parent.m_children.insert(pair<string, ConfigGroup*>(name, this));
}

// destructor
ConfigGroup::~ConfigGroup()
{
}

void ConfigGroup::add_variable(const string& varname, rtest r)
{
    bool inserted = m_variables.insert(pair<string, rtest>(varname, r)).second;
    if(!inserted) {
        config_log.fatal() << "Duplicate ConfigVariable name (" << varname << ") in ConfigGroup '"
                           << m_name << ".'\n\tPlease submit a bug/issue to Astron with your"
                           << " CMakeCache and this ouput.\n";
        exit(1);
    }
}

bool ConfigGroup::validate(ConfigNode node)
{
    if(!node.IsMap()) {
        if(m_name.length() > 0) {
            // NOTE(Kevin): ConfigGroup is used for mappings, not sequences or scalars
            config_log.error() << "Section '" << get_path()
                               << "' has key/value config variables.\n";
        } else {
            // NOTE(Kevin): The root of the config file must be a mapping, not a sequence or scalar
            // P.S.  I'm relatively sure that is what this is checking for, but I could be wrong... Sorry!
            config_log.error() << "Config sections must be at root of config file.\n";
        }
        return false;
    }

    // NOTE(Kevin): Validate each subgroup or variable in this config group
    bool ok = true;
    for(auto it = node.begin(); it != node.end(); ++it) {
        string key = it->first.as<std::string>();

        // Check to see if mapping key is a variable, run its validations if so
        auto found_var = m_variables.find(key);
        if(found_var != m_variables.end()) {
            rtest r = found_var->second;
            if(!r(node)) {
                config_log.info() << "In Section '" << get_path() << "', attribute '"
                                  << key << "' did not match constraint (see error).\n";
                ok = false;
            }
            continue;
        }

        // Check if the key is a subgroup, if so recursively validate it
        auto found_grp = m_children.find(key);
        if(found_grp != m_children.end()) {
            if(!found_grp->second->validate(node[key])) {
                ok = false;
            }
            continue;
        }

        if(m_name.length() > 0) {
            config_log.error() << "Section '" << get_path()
                               << "' has no attribute named '" << key << "'.\n";
        } else {
            config_log.error() << "Section '" << key << "' is not a valid config category.\n";
        }
        ok = false;
    }
    return ok;
}

// constructor
KeyedConfigGroup::KeyedConfigGroup(const string& name, const string& group_key,
                                   ConfigGroup& parent) :
    ConfigGroup(name, parent), m_key(group_key)
{
}

// destructor
KeyedConfigGroup::~KeyedConfigGroup()
{
}

bool KeyedConfigGroup::validate(ConfigNode node)
{
    if(!node.IsMap()) {
        // NOTE(Kevin): KeyedConfigGroup is used for mappings, not sequences or scalars
        config_log.error() << "Section '" << get_path()
                           << "' expects a list of values.\n";
        return false;
    }

    // NOTE(Kevin): A Keyed config group is validated based on the value of a known key;
    // typically the name of this key is "type"
    if(!node[m_key]) {
        config_log.error() << "The section '" << get_path() << "' did not specify the attribute '"
                           << m_key << "'.\n";
        return false;
    }

    // Find the appropriate config group for the value of m_key
    string key = node[m_key].as<std::string>();
    auto found_grp = m_children.find(key);
    if(found_grp == m_children.end()) {
        // A group doesn't exist whose name is the value of m_key
        config_log.error() << "The value '" << key << "' is not valid for attribute '"
                           << m_key << "' of section '" << get_path() << "'.\n";
        print_keys();
        return false;
    }

    ConfigNode child_node = Clone(node);
    child_node.remove(m_key);
    return found_grp->second->validate(child_node);
}

void KeyedConfigGroup::print_keys()
{
    auto out = config_log.info();
    out << "Expected value in '" << m_name << "',\n"
        << "    Candidates for attribute '" << m_key << "' are:\n";
    for(auto it = m_children.begin(); it != m_children.end(); ++it) {
        out << "        " << it->second->get_name() << "\n";
    }
    out << "\n";
}


ConfigList::ConfigList(const string& name, ConfigGroup& parent) : ConfigGroup(name, parent)
{
}

ConfigList::~ConfigList()
{
}

bool ConfigList::validate(ConfigNode node)
{
    if(!node.IsSequence()) {
        // NOTE(Kevin): ConfigList is used for sequences, not mappings or scalars
        config_log.error() << "Section '" << get_path()
                           << "' expects a list of values.\n";
        return false;
    }

    bool ok = true;
    for(auto it = node.begin(); it != node.end(); ++it) {
        if(!ConfigGroup::validate(*it)) {
            ok = false;
        }
    }

    return ok;
}

// constructor
KeyedConfigList::KeyedConfigList(const string& name, const string& list_key, ConfigGroup& parent) :
    KeyedConfigGroup(name, list_key, parent)
{
}

// destructor
KeyedConfigList::~KeyedConfigList()
{
}

bool KeyedConfigList::validate(ConfigNode node)
{
    if(!node.IsSequence()) {
        // NOTE(Kevin): KeyedConfigList is used for sequences, not mappings or scalars
        config_log.error() << "Section '" << get_path()
                           << "' expects a list of values.\n";
        return false;
    }

    int n = -1;
    bool ok = true;
    for(auto it = node.begin(); it != node.end(); ++it) {
        n += 1;

        // Items in a keyed config list must be a mapping include a key with the name from m_key
        if(!it->IsMap()) {
            config_log.error() << "The " << n << "th item of section '" << get_path()
                               << "' does not have key/value config variables.\n";
            ok = false;
            continue;
        }

        // NOTE(Kevin): A Keyed config list is validated based on a given subkey
        // in an items values; typically the name of this key "type"
        ConfigNode element = *it;
        if(!element[m_key]) {
            config_log.error() << "The " << n << "th item of section '" << get_path()
                               << "' did not specify the attribute '" << m_key << "'.\n";
            ok = false;
            continue;
        }

        string key = element[m_key].as<std::string>();
        auto found_grp = m_children.find(key);
        if(found_grp == m_children.end()) {
            config_log.error() << "The value '" << key << "' is not valid for attribute '"
                               << m_key << "' of section '" << get_path() << "'.\n";
            print_keys();
            ok = false;
            continue;
        }

        if(!found_grp->second->validate(element)) {
            ok = false;
        }
    }

    return ok;
}
