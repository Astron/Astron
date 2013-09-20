#include "config.h"
#include <exception>
#include <stdexcept>
#include <sstream>

bool ConfigFile::load(std::istream &is)
{
	m_node = YAML::Load(is);
	if(m_node.IsNull())
	{
		return false;
	}
	return true;
}

YAML::Node ConfigFile::copy_node()
{
	return YAML::Clone(m_node);
}
