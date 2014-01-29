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
			config_log.error() << "Category '" << m_path
			                    << "' has key/value config variables.\n";
		}
		else
		{
			config_log.error() << "Config categories must be at root of config file.\n";
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
				config_log.error() << "Category '" << m_path
				                    << "' has no attribute named '" << key << "'.\n";
			}
			else
			{
				config_log.error() << "Category '" << key << "' is not a valid config category.\n";
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
		config_log.error() << "Category '" << m_path
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
	return true;
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
