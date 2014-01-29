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
		T m_def_val;

		// get_val_by_path gets the value at the given config path
		T get_val_by_path(ConfigNode node, std::string path)
		{
			size_t offset = 0;
			ConfigNode cnode = YAML::Clone(node);
			bool going = true;
			while(going)
			{
				size_t noffset = path.find("/", offset);
				if(noffset == std::string::npos)
				{
					going = false;
					break;
				}
				else
				{
					cnode = cnode[path.substr(offset, noffset - offset)];
					if(!cnode.IsDefined())
					{
						return m_def_val;
					}
				}
				offset = noffset + 1;
			}
			cnode = cnode[path.substr(offset, path.length() - offset)];
			if(!cnode.IsDefined())
			{
				return m_def_val;
			}
			return cnode.as<T>();
		}

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
		}

		// Get rval gets the value of the config variable relative to the variable's ConfigGroup.
		T get_rval(ConfigNode node)
		{
			return get_val_by_path(node, m_name);
		}

		// Get val gets the value of the config variable from the global config scope.
		T get_val()
		{
			return get_val_by_path(g_config->m_node, m_group->get_path() + m_name);
		}

};
