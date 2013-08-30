#pragma once
#include <istream>
#include <yaml-cpp/yaml.h>

class ConfigFile
{
	public:
		bool load(std::istream &is);
	private:
		YAML::Node m_node;
};
