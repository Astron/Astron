#pragma once
#include <string>          // std::string
#include <unordered_map>   // std::unordered_map
#include <unordered_set>   // std::unordered_set
#include <functional>      // std::function
#include <yaml-cpp/yaml.h> // YAML::Node

typedef YAML::Node ConfigNode;
typedef std::function<bool(ConfigNode)> rtest;
typedef std::function<bool()> test;

void config_error(const std::string& msg);

class ConfigGroup
{
    template <typename T>
    friend class ConfigVariable;

  public:
    static ConfigGroup& root();

    ConfigGroup(const std::string& name, ConfigGroup& parent = root());
    virtual ~ConfigGroup();

    // get_name returns the name of the ConfigGroup.
    inline std::string get_name() const
    {
        return m_name;
    }

    // get_path returns the name of this group and each parent on the way
    //     to the root separated by slashes, with the oldest ancestor at the beginning.
    inline std::string get_path() const
    {
        return m_path;
    }

    // get_child_node returns the sub node of a given ConfigNode that
    //     corresponds with a given child group of this Group.
    inline ConfigNode get_child_node(std::string grp_name, ConfigNode node)
    {
        return node[grp_name];
    }
    inline ConfigNode get_child_node(ConfigGroup grp, ConfigNode node)
    {
        return node[grp.m_name];
    }

    virtual bool validate(ConfigNode node);

  protected:
    ConfigGroup* m_parent;
    std::string m_name, m_path;

    std::unordered_map<std::string, rtest> m_variables;
    std::unordered_map<std::string, ConfigGroup*> m_children;

  private:
    ConfigGroup(); // root constructor
    void add_variable(const std::string&, rtest);
};

class ConfigList : public ConfigGroup
{
  public:
    ConfigList(const std::string& name, ConfigGroup& parent = ConfigGroup::root());
    virtual ~ConfigList();

    virtual bool validate(ConfigNode node);
};

class KeyedConfigList : public ConfigGroup
{
  public:
    KeyedConfigList(const std::string& name, const std::string &list_key,
                    ConfigGroup& parent = ConfigGroup::root());
    virtual ~KeyedConfigList();

    virtual bool validate(ConfigNode node);

  private:
    std::string m_key;

    // print_keys outputs all valid keys for the list into
    //     the Config log with Info severity.
    void print_keys();
};
