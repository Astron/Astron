#pragma once
#include <yaml-cpp/yaml.h> // YAML::Node
#include <istream>         // std::istream

typedef YAML::Node ConfigNode;

template<class T>
class ConfigVariable
{
	private:
		std::string m_name;
		T m_def_val;

	public:
		ConfigVariable(ConfigGroup& grp, const std::string &name, const T& def_val);

		// Get rval gets the value of the config variable relative to a given ConfigGroup.
		T get_rval(ConfigNode node);

		// Get val gets the value of the config variable from the global config scope.
		T get_val();
};

class ConfigFile
{
	public:
		bool load(std::istream &is);

		ConfigNode copy_node();

	private:
		template <class T>
		friend class ConfigVariable;

		ConfigNode m_node;
};

class ConfigGroup
{
	public:
		static ConfigGroup root;

		ConfigGroup(ConfigGroup& parent);
};

extern ConfigFile *g_config;

typedef ConfigNode RoleConfig;
typedef ConfigNode RangesConfig;
typedef ConfigNode DBBackendConfig;
