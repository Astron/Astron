#include "config.h"

bool ConfigFile::load(std::istream &is)
{
	m_node = YAML::Load(is);
	return !m_node.IsNull(); // TBD
}
