#include "global.h"
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

template<class T>
ConfigVariable<T>::ConfigVariable(const std::string &name, const T& def_val) : m_name(name), m_def_val(def_val)
{
}

template<class T>
T ConfigVariable<T>::get_val(YAML::Node node)
{
	size_t offset = 0;
	YAML::Node cnode = node;
	bool going = true;
	while(going)
	{
		size_t noffset = m_name.find("/", offset);
		if(noffset == std::string::npos)
		{
			going = false;
			break;
		}
		else
		{
			cnode = cnode[m_name.substr(offset, noffset-offset)];
			if(cnode.IsNull())
			{
				std::stringstream ss;
				ss << "Config node " << m_name << "does not exist.";
				throw std::logic_error(ss.str());
			}
		}
		offset = noffset+1;
	}
	cnode = cnode[m_name.substr(offset, m_name.length()-offset)];
	return cnode.as<T>();
}

template<class T>
T ConfigVariable<T>::get_val()
{
	return get_val(gConfig->m_node)
}