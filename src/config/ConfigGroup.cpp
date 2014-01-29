#include "ConfigGroup.h"
#include "core/Logger.h"
using namespace std;

LogCategory config_log("config", "Config");

// root constructor
ConfigGroup::ConfigGroup() : m_name(""), m_path("")
{
}

// constructor
ConfigGroup::ConfigGroup(const string& name, ConfigGroup& parent) : m_parent(&parent)
{
	m_name = name;
	m_path = parent.get_path() + name + "/";
	parent.m_children.insert(pair<string, ConfigGroup*>(name, this));
}

// destructor
ConfigGroup::~ConfigGroup()
{
}

bool ConfigGroup::validate(ConfigNode node)
{
	if(!node.IsMap())
	{
		if(m_name.length() > 0)
		{
			config_log.error() << "Section '" << m_path
			                    << "' has key/value config variables.\n";
		}
		else
		{
			config_log.error() << "Config sections must be at root of config file.\n";
		}
		return false;
	}

	bool ok = true;
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		string key = it->first.as<std::string>();
		auto found_var = m_variables.find(key);
		auto found_grp = m_children.find(key);
		if(found_var != m_variables.end())
		{
			// TODO: Test var contraints
		}
		else if(found_grp != m_children.end())
		{
			if(!found_grp->second->validate(node[key]))
			{
				ok = false;
			}
		}
		else
		{
			if(m_name.length() > 0)
			{
				config_log.error() << "Section '" << m_path
				                    << "' has no attribute named '" << key << "'.\n";
			}
			else
			{
				config_log.error() << "Section '" << key << "' is not a valid config category.\n";
			}
			ok = false;
		}
	}
	return ok;
}

ConfigList::ConfigList(const string& name, ConfigGroup& parent) : ConfigGroup(name, parent)
{
}

ConfigList::~ConfigList()
{
}

bool ConfigList::validate(ConfigNode node)
{
	if(!node.IsSequence())
	{
		config_log.error() << "Sec '" << m_path
		                   << "' expects a list of values.\n";
		return false;
	}

	bool ok = true;
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		if(!ConfigGroup::validate(*it))
		{
			ok = false;
		}
	}

	return ok;
}

// constructor
KeyedConfigList::KeyedConfigList(const string& name, const string& list_key, ConfigGroup& parent) :
	ConfigGroup(name, parent), m_key(list_key)
{
}

// destructor
KeyedConfigList::~KeyedConfigList()
{
}

bool KeyedConfigList::validate(ConfigNode node)
{
	if(!node.IsSequence())
	{
		config_log.error() << "Section '" << m_path
		                   << "' expects a list of values.\n";
		return false;
	}

	int n = -1;
	bool ok = true;
	for(auto it = node.begin(); it != node.end(); ++it)
	{
		n += 1;

		if(!it->IsMap())
		{
			config_log.error() << "The " << n << "th item of section '" << m_path
			                   << "' does not have key/value config variables.\n";
			ok = false;
			continue;
		}

		ConfigNode element = *it;
		if(!element[m_key])
		{
			config_log.error() << "The " << n << "th item of section '" << m_path
			                   << "' did not specify the attribute '" << m_key << "'.\n";
			ok = false;
			continue;
		}

		string key = element[m_key].as<std::string>();
		auto found_grp = m_children.find(key);
		if(found_grp == m_children.end())
		{
			config_log.error() << "The value '" << key << "' is not valid for attribute '"
			                   << m_key << "' of section '" << m_path << "'.\n";
			ok = false;
			continue;
		}

		if(!found_grp->second->validate(element))
		{
			ok = false;
		}
	}

	return ok;
}

KeyedConfigGroup::KeyedConfigGroup(const string& key, KeyedConfigList& list) :
	ConfigGroup(key, list)
{
}

KeyedConfigGroup::~KeyedConfigGroup()
{
}

bool KeyedConfigGroup::validate(ConfigNode node)
{
	return true;
}
