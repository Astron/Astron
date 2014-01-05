// Filename: File.cpp
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "File.h"
#include "Class.h"
#include "Struct.h"
#include "Field.h"
#include "DistributedType.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


typedef std::unordered_map<std::string, DistributedType*>::value_type TypeName;

// constructor
File::File()
{
}

//destructor
File::~File()
{
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		delete(*it);
	}
	for(auto it = m_structs.begin(); it != m_classes.end(); ++it)
	{
		delete(*it);
	}
	for(auto it = m_imports.begin(); it != m_imports.end(); ++it)
	{
		delete(*it);
	}

	m_classes.clear();
	m_structs.clear();
	m_imports.clear();
	m_types_by_id.clear();
	m_types_by_name.clear();
	m_fields_by_id.clear();
	m_keywords.clear();
}

// add_class adds the newly-allocated class to the file.
//     Returns false if there is a name conflict.
//     The File becomes the owner of the pointer and will delete it when it destructs.
bool File::add_class(Class *cls)
{
	// Classes have to have a name
	if(cls->get_name().empty())
	{
		return false;
	}

	// A Class can't share a name with any other type.
	bool inserted = m_types_by_name.insert(TypeName(cls->get_name(), cls).second;
	if(!inserted)
	{
		return false;
	}

	cls->set_id(m_types_by_id.size());
	m_types_by_id.push_back(cls);
	m_classes.push_back(cls);
	return true;
}

// add_struct adds the newly-allocated struct to the file.
//     Returns false if there is a name conflict.
//     The File becomes the owner of the pointer and will delete it when it destructs.
bool File::add_struct(Struct *srct)
{
	// Structs have to have a name
	if(strct->get_name().empty())
	{
		return false;
	}

	// A Struct can't share a name with any other type.
	bool inserted = m_types_by_name.insert(TypeName(strct->get_name(), strct).second;
	if(!inserted)
	{
		return false;
	}

	strct->set_id(m_types_by_id.size());
	m_types_by_id.push_back(strct);
	m_classes.push_back(strct);
	return true;
}

// add_typedef adds the alias <name> to the file for the type <type>.
//     Returns false if there is a name conflict.
bool File::add_typedef(const std::string& name, DistributedType* type)
{
	// Typedefs can't use the empty string as a name
	if(name.empty())
	{
		return false;
	}

	// A type alias can't share a name with any other type.
	return m_types_by_name.insert(TypeName(name, type)).second;
}

// add_import adds a newly-allocated import to the file.
//     Imports with duplicate modules are combined.
void File::add_import(Import* import)
{
	// TODO: Combine duplicates
	m_imports.push_back(import);
}

// add_keyword adds a keyword with the name <keyword> to the list of declared keywords.
void File::add_keyword(const std::string &keyword)
{
	if(!has_keyword(keyword))
	{
		m_keywords.push_back(keyword);
	}
}

// add_field gives the field a unique id within the file.
void File::add_field(Field *field)
{
	field->set_id((unsigned int)m_fields_by_id.size());
	m_fields_by_id.push_back(field);
}

// update_inheritance updates the field inheritance of all classes inheriting from <dclass>.
void File::update_inheritance(Class* dclass)
{
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		Class* cls = (*it)->as_class();
		if(cls != (Class*)NULL)
		{
			size_t num_parents = cls->get_num_parents();
			for(unsigned int i = 0; i < num_parents; ++i)
			{
				if(cls->get_parent(i) == dclass)
				{
					cls->rebuild_fields();
					break;
				}
			}
		}
	}
}

// rebuild_inherited_fields reconstructs the inherited fields table of all classes.
void File::rebuild_inherited_fields()
{
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		Class* cls = (*it)->as_class();
		if(cls != (Class*)NULL)
		{
			cls->rebuild_fields();
		}
	}
}

uint32_t Hashable::get_hash() const
{
	HashGenerator hashgen;
	generate_hash(hashgen);
	return hashgen.get_hash();
}

// generate_hash accumulates the properties of this file into the hash.
void File::generate_hash(HashGenerator& hashgen) const
{
	hashgen.add_int(m_classes.size());
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}

	hashgen.add_int(m_structs.size());
	for(auto it = m_structs.begin(); it != m_structs.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}

	hashgen.add_int(m_keywords.size());
	for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
	{
		hashgen.add_string(*it);
	}
}


} // close namespace dclass