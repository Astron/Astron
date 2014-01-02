// Filename: Class.cpp
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Class.h"
#include "File.h"
#include "HashGenerator.h"

#include "indent.h"

#include <algorithm>     // std::sort
#include <unordered_set> // std::unordered_set
using namespace std;
namespace dclass   // open namespace
{

inline bool sort_fields_by_id(const Field* a, const Field* b)
{
	return (a->get_id()  <  b->get_id());
}

// constructor
Class::Class(File* file, const string &name) : Struct(file, name), m_constructor(NULL)
{
	m_constructor = NULL;
}

// destructor
Class::~Class()
{
	if(m_constructor != (Field*)NULL)
	{
		delete m_constructor;
	}
}

// as_class returns the same pointer converted to a class pointer
//     if this is in fact a class; otherwise, returns NULL.
Class* Class::as_class()
{
	return this;
}
const Class* Class::as_class() const
{
	return this;
}

// rebuild_fields recomputes the list of inherited fields for the class.
void Class::rebuild_fields()
{
	unordered_set<string> names;

	m_fields.clear();
	m_fields_by_name.clear();
	m_fields_by_id.clear();
	m_bytesize = 0;
	m_has_fixed_size = true;

	// First, all of the inherited fields from our parent are at the top of the list.
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		const Class *parent = (*it);
		size_t num_inherited_fields = parent->get_num_fields();
		for(unsigned int i = 0; i < num_inherited_fields; ++i)
		{
			Field *field = parent->get_field(i);
			if(!field->get_name().empty())
			{
				bool inserted = names.insert(field->get_name()).second;
				if(inserted)
				{
					// The earlier parent shadows the later parent.
					m_fields.push_back(field);
					m_fields_by_name.insert(unordered_map<string, Field*>::value_type(field->get_name(), field));
					if(m_file != (File*)NULL)
					{
						m_fields_by_id.insert(unordered_map<int, Field*>::value_type(field->get_id(), field));
					}

					if(!field->as_molecular_field())
					{
						m_bytesize += field->get_size();
						m_has_fixed_size = m_has_fixed_size && field->has_fixed_size();
					}
				}
			}
		}
	}

	// Now add the local fields at the end of the list.
	// If any fields n this list were already defined by a parent,
	// we will shadow the parent definition (ie. remove the parent's field).
	for(auto it = m_base_fields.begin(); it != m_base_fields.end(); ++it)
	{
		Field *field = (*it);

		bool inserted = names.insert(field->get_name()).second;
		if(!inserted)
		{
			// This local field shadows an inherited field.
			// Remove the parent's field from our list.
			for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
			{
				Field *shadowed = (*it);
				if(shadowed->get_name() == field->get_name())
				{
					m_fields.erase(it);
					m_fields_by_id.erase(shadowed->get_id());
					m_fields_by_name.erase(shadowed->get_name());
					return;
				}
			}
		}

		// Now add the local field.
		m_fields.push_back(field);
		m_fields_by_name.insert(unordered_map<string, Field*>::value_type(field->get_name(), field));
		if(m_file != (File*)NULL)
		{
			m_fields_by_id.insert(unordered_map<int, Field*>::value_type(field->get_id(), field));
		}
		if(!field->as_molecular_field())
		{
			m_bytesize += field->get_size();
			m_has_fixed_size = m_has_fixed_size && field->has_fixed_size();
		}
	}

	// The fields must be sorted by id
	sort(m_fields.begin(), m_fields.end(), sort_fields_by_id);

	if(!m_has_fixed_size)
	{
		m_bytesize = 0;
	}
}


// add_field adds the newly-allocated field to the class.  The class becomes
//     the owner of the pointer and will delete it when it destructs.
//     Returns true if the field is successfully added, or false if the field cannot be added.
bool Class::add_field(Field *field)
{
	// Classes can't share fields.
	if(field->get_class() != NULL && field->get_class() != this)
	{
		return false;
	}

	// Class fields must have names
	if(field->get_name().empty())
	{
		return false;
	}

	// If the field has the same name as the class, it is a constructor
	if(field->get_name() == m_name)
	{
		// Make sure we don't already have a constructor
		if(m_constructor != (Field*)NULL)
		{
			return false;
		}

		// The constructor must be an atomic field.
		if(field->as_atomic_field() == (AtomicField*)NULL)
		{
			return false;
		}

		field->set_class(this);
		m_constructor = field;
		m_fields_by_name.insert(unordered_map<string, Field*>::value_type(field->get_name(), field));
		return true;
	}

	// Try to add the field as a normal field
	if(!Struct::add_field(field))
	{
		return false; // But it failed
	}

	// A Class has to keep track of which fields are not inherted.
	m_base_fields.push_back(field);

	// Also, tell the file to update any subclasses of this class.
	if(m_file != (File*)NULL)
	{
		m_file->update_inheritance(this);
	}

	return true;
}

// add_parent adds a new parent to the inheritance hierarchy of the class.
//     Note: This is normally called only during parsing.
void Class::add_parent(Class *parent)
{
	m_parents.push_back(parent);
	rebuild_fields();
	m_file->update_inheritance(this);
}


} // close namespace dclass
