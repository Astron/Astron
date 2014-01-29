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
	return true;
}

// constructor
ConfigList::ConfigList(const string& name, const string& list_key, ConfigGroup& parent) :
	ConfigGroup(name, parent), m_key(list_key)
{
}

// destructor
ConfigList::~ConfigList()
{
}

void ConfigList::add_element_group(ConfigGroup* element)
{
	m_element_groups.insert(pair<string, ConfigGroup*>(element->get_name(), element));
}

bool ConfigList::validate(ConfigNode node)
{
	return true;
}
