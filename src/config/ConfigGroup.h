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

/* The base ConfigGroup is a "mapping" of config values. Each key in the
   mapping can correspond to a ConfigVariable or another ConfigGroup (or subclass).

   Other collections of config variables inherit from ConfigGroup

   Example usage:

        general:  # ConfigGroup("general")>

            eventlogger: 127.0.0.1:9090  # ConfigVariable<string>("eventlogger", general_group)

            dc_files:  # ConfigVariable<vector<string>>("dc_files", general_group)
                - core.dc
                - game.dc
                - minigames.dc
*/
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
        return (m_parent == nullptr) ? "" : (m_parent->get_path() + m_name + "/");
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
    std::string m_name;
    ConfigGroup* m_parent;

    std::unordered_map<std::string, rtest> m_variables;
    inline std::unordered_map<std::string, ConfigGroup*>& get_children()
    {
        return config_tree()[this];
    }

  private:
    ConfigGroup(); // root constructor
    void add_variable(const std::string&, rtest);

    static std::unordered_map<ConfigGroup*, std::unordered_map<std::string, ConfigGroup*>>&
            config_tree();
};


/* A KeyedConfigGroup is a "mapping" of ConfigGroups, where each ConfigGroup's
   variables are flattened into the parent KeyedConfigGroup, and which grouping
   of ConfigVariables to use is detected by a known key in the subgroups.

   A KeyedConfigGroup is useful if you have a factory which is expected to be
   configured to instantiate a single resulting object.

   Example usage:

        backend:  # KeyedConfigGroup("backend", "type")
            - type: "mongodb"  # ConfigGroup("mongodb", backend_group)
              A: 3

        backend
            - type: "mysql"  # ConfigGroup("mysql", backend_group)
              B: "Tasty food"
 */
class KeyedConfigGroup : public ConfigGroup
{
  public:
    KeyedConfigGroup(const std::string& name, const std::string& group_key,
                     ConfigGroup& parent = ConfigGroup::root(), const std::string& default_key = "");
    virtual ~KeyedConfigGroup();

    bool validate(ConfigNode node) override;

  protected:
    std::string m_key;
    std::string m_default_key;

    // print_keys outputs all valid keys for the list into
    //     the Config log with Info severity.
    void print_keys();
};


/* A ConfigList is a "sequence" of ConfigGroups, where each mapping in the
   sequence is expected to share the same config variables.

   For sequences of scalar values use ConfigVariable<vector<ScalarType>>

   Example usage:

        uberdogs:  # ConfigList("uberdogs")

          - id: 1234             # ConfigVariable("id", uberdogs_group)
            class: LoginManager  # ConfigVariable("class", uberdogs_group)
            anonymous: true      # ConfigVariable("anonymous", uberdogs_group)

          - id: 1235
            class: KillManager
            anonymous: false
*/
class ConfigList : public ConfigGroup
{
  public:
    ConfigList(const std::string& name, ConfigGroup& parent = ConfigGroup::root());
    virtual ~ConfigList();

    bool validate(ConfigNode node) override;
};


/* A KeyedConfigList is a "sequence" of ConfigGroups, where each mapping in the
   sequence is expected to have different ConfigVariables as detected by a
   known key in the subgroups.

   A KeyedConfigList is useful if you have a factory which is expected to be
   configured to instantiate multiple different objects.

   Example usage:

        roles:  # KeyedConfigList("roles", "type")

            - type: "ConfigGroupAType"  # ConfigGroup("ConfigGroupAType", roles_group)
              A: 3

            - type: "ConfigGroupBType"  # ConfigGroup("ConfigGroupBType", roles_group)
              B: "Tasty food"
 */
class KeyedConfigList : public KeyedConfigGroup
{
  public:
    KeyedConfigList(const std::string& name, const std::string &list_key,
                    ConfigGroup& parent = ConfigGroup::root());
    virtual ~KeyedConfigList();

    bool validate(ConfigNode node) override;
};
