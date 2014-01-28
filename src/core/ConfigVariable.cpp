#include "config.h"
using namespace std;

template<class T>
ConfigVariable::ConfigVariable(ConfigGroup& grp, const string &name, const T& def_val) :
	m_name(name), m_def_val(def_val)
{
}

template<class T>
T get_rval(YAML::Node node)
{
	size_t offset = 0;
	YAML::Node cnode = YAML::Clone(node);
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
			cnode = cnode[m_name.substr(offset, noffset - offset)];
			if(!cnode.IsDefined())
			{
				return m_def_val;
			}
		}
		offset = noffset + 1;
	}
	cnode = cnode[m_name.substr(offset, m_name.length() - offset)];
	if(!cnode.IsDefined())
	{
		return m_def_val;
	}
	return cnode.as<T>();
}

template<class T>
T ConfigVariable::get_val()
{
	return get_rval(g_config->m_node);
}
