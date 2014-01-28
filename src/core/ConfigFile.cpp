#include "config.h"
#include <sstream> // std::istream
using namespace std;

bool ConfigFile::load(istream &is)
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
