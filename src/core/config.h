#pragma once
#include <istream>
#include <yaml-cpp/yaml.h>

class ConfigFile
{
public:
	bool load(std::istream &is);
	YAML::Node copy_node();
private:
	template <class T>
	friend class ConfigVariable;
	YAML::Node m_node;
};

extern ConfigFile *gConfig;

typedef YAML::Node RoleConfig;
typedef YAML::Node DBEngineConfig;

template<class T>
class ConfigVariable
{
private:
	std::string m_name;
	T m_def_val;
public:
	ConfigVariable(const std::string &name, const T& def_val) : m_name(name), m_def_val(def_val)
	{
	}

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
				cnode = cnode[m_name.substr(offset, noffset-offset)];
				if(!cnode.IsDefined())
				{
					return m_def_val;
				}
			}
			offset = noffset+1;
		}
		cnode = cnode[m_name.substr(offset, m_name.length()-offset)];
		if(!cnode.IsDefined())
		{
			return m_def_val;
		}
		return cnode.as<T>();
	}

	T get_val()
	{
		return get_rval(gConfig->m_node);
	}
};