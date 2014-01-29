#include "configGroups.h"
using namespace std;

ConfigGroup::ConfigGroup() : m_name(""), m_path("")
{
}

ConfigGroup::ConfigGroup(const string& name, ConfigGroup& parent) : m_parent(&parent)
{
	m_name = name;
	m_path = parent.get_path() + name + "/";
	parent.m_children.insert(pair<string, ConfigGroup*>(name, this));
}

ConfigList::ConfigList(const string& name, ConfigGroup& parent) : ConfigGroup(name, parent)
{
}

void ConfigList::add_element_group(ConfigGroup* element)
{
	m_element_groups.insert(pair<string, ConfigGroup*>(element->get_name(), element));
}
