#pragma once
#include <istream>
#include <yaml-cpp/yaml.h>

class ConfigFile
{
	public:
		bool load(std::istream &is);
	private:
		template <class T>
		friend class ConfigVariable;
		YAML::Node m_node;
};

typedef YAML::Node RoleConfig;

template<class T>
class ConfigVariable
{
	public:
		ConfigVariable(const std::string &name, const T& def_val);
		T get_val(YAML::Node node);
		T get_val();
	private:
		T m_def_val;
		std::string m_name;
};