#pragma once
#include <string>          // std::string
#include <unordered_map>   // std::unordered_map
#include <unordered_set>   // std::unordered_set

class ConfigGroup
{
	template <typename T>
	friend class ConfigVariable;

	public:
		static ConfigGroup root;

		ConfigGroup(); // root constructor
		ConfigGroup(const std::string& name, ConfigGroup& parent = root);

		inline std::string get_name() const
		{
			return m_name;
		}
		inline std::string get_path() const
		{
			return m_path;
		}

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
		ConfigList(const std::string& name, ConfigGroup& parent = ConfigGroup::root);

		void add_element_group(ConfigGroup* element);

	private:
		std::unordered_map<std::string, ConfigGroup*> m_element_groups;
};
