#pragma once
#include <string>          // std::string
#include <unordered_map>   // std::unordered_map
#include <unordered_set>   // std::unordered_set
#include <yaml-cpp/yaml.h> // YAML::Node

typedef YAML::Node ConfigNode;

// todo: get rid of
typedef ConfigNode RangesConfig;
typedef ConfigNode DBBackendConfig;

class ConfigGroup
{
	template <typename T>
	friend class ConfigVariable;

	public:
		static ConfigGroup root;

		ConfigGroup(); // root constructor
		ConfigGroup(const std::string& name, ConfigGroup& parent = root);
		virtual ~ConfigGroup();

		inline std::string get_name() const
		{
			return m_name;
		}
		inline std::string get_path() const
		{
			return m_path;
		}

		virtual bool validate(ConfigNode node);

	protected:
		ConfigGroup* m_parent;
		std::string m_name, m_path;
		std::unordered_set<std::string> m_variables;
		std::unordered_map<std::string, ConfigGroup*> m_children;

	private:
};

class ConfigList : public ConfigGroup
{
	public:
		ConfigList(const std::string& name, const std::string &list_key,
		           ConfigGroup& parent = ConfigGroup::root);
		virtual ~ConfigList();

		void add_element_group(ConfigGroup* element);
		virtual bool validate(ConfigNode node);

	private:
		std::string m_key;
		std::unordered_map<std::string, ConfigGroup*> m_element_groups;
};
