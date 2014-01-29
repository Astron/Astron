#pragma once
#include "ConfigGroup.h"
#include <istream> // std::istream

class ConfigFile
{
	template<typename T>
	friend class ConfigVariable;

	public:
		bool load(std::istream &is);

		ConfigNode copy_node();

	private:
		ConfigNode m_node;
};
extern ConfigFile *g_config;

template<typename T>
class ConfigVariable
{
	private:
		ConfigGroup* m_group;
		std::string m_name;
		std::string m_path;
		T m_def_val;

	public:
		ConfigVariable(const std::string& name, const T& def_val,
		               ConfigGroup& grp = ConfigGroup::root) :
			m_name(name), m_def_val(def_val), m_group(&grp)
		{
			bool inserted = grp.m_variables.insert(name).second;
			if(!inserted)
			{
				// TODO: Produce a warning for the developer or something...
			}

			m_path = grp.get_path() + name;
		}

		// Get rval gets the value of the config variable relative to a given ConfigGroup.
		T get_rval(ConfigNode node)
		{
			size_t offset = 0;
			ConfigNode cnode = YAML::Clone(node);
			bool going = true;
			while(going)
			{
				size_t noffset = m_path.find("/", offset);
				if(noffset == std::string::npos)
				{
					going = false;
					break;
				}
				else
				{
					cnode = cnode[m_path.substr(offset, noffset - offset)];
					if(!cnode.IsDefined())
					{
						return m_def_val;
					}
				}
				offset = noffset + 1;
			}
			cnode = cnode[m_path.substr(offset, m_path.length() - offset)];
			if(!cnode.IsDefined())
			{
				return m_def_val;
			}
			return cnode.as<T>();
		}

		// Get val gets the value of the config variable from the global config scope.
		T get_val()
		{
			return get_rval(g_config->m_node);
		}

};
