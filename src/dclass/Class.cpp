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
#include "AtomicField.h"
#include "HashGenerator.h"
#include "File.h"

#include "indent.h"

#include <algorithm>
#include <unordered_set>
namespace dclass   // open namespace
{


class SortFieldsByIndex
{
	public:
		inline bool operator()(const Field* a, const Field* b) const
		{
			return a->get_id() < b->get_id();
		}
};

// constructor
Class::Class(File* file, const std::string &name) : Struct(file, name), m_constructor(NULL)
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

/* TODO: Move and update appropriately
// output formats a string representation of the class in .dc file syntax
//     as "dclass IDENTIFIER" or "struct IDENTIFIER" with structs having optional IDENTIFIER,
//     and outputs the formatted string to the stream.
void Class::output(std::ostream &out) const
{
	if(m_is_struct)
	{
		out << "struct";
	}
	else
	{
		out << "dclass";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}
}

// output formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Class::output(std::ostream &out, bool brief) const
{
	output_instance(out, brief, "", "", "");
}

// write formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Class::write(std::ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level);
	if(m_is_struct)
	{
		out << "struct";
	}
	else
	{
		out << "dclass";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}

	if(!m_parents.empty())
	{
		auto it = m_parents.begin();
		out << " : " << (*it)->m_name;
		++it;
		while(it != m_parents.end())
		{
			out << ", " << (*it)->m_name;
			++it;
		}
	}

	out << " {";
	if(!brief && m_number >= 0)
	{
		out << "  // index " << m_number;
	}
	out << "\n";

	if(m_constructor != (Field*)NULL)
	{
		m_constructor->write(out, brief, indent_level + 2);
	}

	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->write(out, brief, indent_level + 2);

		
		//if (true || (*fi)->has_default_value()) {
		//  indent(out, indent_level + 2) << "// = ";
		//  Packer packer;
		//  packer.set_unpack_data((*fi)->get_default_value());
		//  packer.begin_unpack(*fi);
		//  packer.unpack_and_format(out, false);
		//  if (!packer.end_unpack()) {
		//    out << "<error>";
		//  }
		//  out << "\n";
		//}
	}

	indent(out, indent_level) << "};\n";
}

// output_instance formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Class::output_instance(std::ostream &out, bool brief, const std::string &prename,
                            const std::string &name, const std::string &postname) const
{
	if(m_is_struct)
	{
		out << "struct";
	}
	else
	{
		out << "dclass";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}

	if(!m_parents.empty())
	{
		auto it = m_parents.begin();
		out << " : " << (*it)->m_name;
		++it;
		while(it != m_parents.end())
		{
			out << ", " << (*it)->m_name;
			++it;
		}
	}

	out << " {";

	if(m_constructor != (Field*)NULL)
	{
		m_constructor->output(out, brief);
		out << "; ";
	}

	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->output(out, brief);
		out << "; ";
	}

	out << "}";
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
}
*/

// rebuild_inherited_fields recomputes the list of inherited fields for the class.
void Class::rebuild_inherited_fields()
{
	std::unordered_set<std::string> names;

	m_inherited_fields.clear();

	// First, all of the inherited fields from our parent are at the top of the list.
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		const Class *parent = (*it);
		size_t num_inherited_fields = parent->get_num_inherited_fields();
		for(unsigned int i = 0; i < num_inherited_fields; ++i)
		{
			Field *field = parent->get_inherited_field(i);
			if(!field->get_name().empty())
			{
				bool inserted = names.insert(field->get_name()).second;
				if(inserted)
				{
					// The earlier parent shadows the later parent.
					m_inherited_fields.push_back(field);
				}
			}
		}
	}

	// Now add the local fields at the end of the list.  If any fields
	// in this list were already defined by a parent, we will shadow the
	// parent definition (that is, remove the parent's field from our
	// list of inherited fields).
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		Field *field = (*it);
		if(field->get_name().empty())
		{
			m_inherited_fields.push_back(field);
		}
		else
		{
			bool inserted = names.insert(field->get_name()).second;
			if(!inserted)
			{
				// This local field shadows an inherited field.
				// Remove the parent's field from our list.
				for(auto it = m_inherited_fields.begin(); it != m_inherited_fields.end(); ++it)
				{
					Field *shadow = (*it);
					if(shadow->get_name() == field->get_name())
					{
						m_inherited_fields.erase(it);
						return;
					}
				}
			}

			// Now add the local field.
			m_inherited_fields.push_back(field);
		}
	}

	sort(m_inherited_fields.begin(), m_inherited_fields.end(), SortFieldsByIndex());
}

/*
// add_field adds the newly-allocated field to the class.  The class becomes the
//     owner of the pointer and will delete it when it destructs.
//     Returns true if the field is successfully added,
//     or false if there was a name conflict or some other problem.
bool Class::add_field(Field *field)
{
	assert(field->get_class() == this || field->get_class() == NULL);
	field->set_class(this);
	if(m_file != (File*)NULL)
	{
		m_file->mark_inherited_fields_stale();
	}

	if(!field->get_name().empty())
	{
		if(field->get_name() == m_name)
		{
			// This field is a constructor.
			if(m_constructor != (Field*)NULL)
			{
				// We already have a constructor.
				return false;
			}
			if(field->as_atomic_field() == (AtomicField*)NULL)
			{
				// The constructor must be an atomic field.
				return false;
			}
			m_constructor = field;
			m_fields_by_name.insert(
				std::unordered_map<std::string, Field*>::value_type(field->get_name(), field));
			return true;
		}

		bool inserted = m_fields_by_name.insert(
			std::unordered_map<std::string, Field*>::value_type(field->get_name(), field)).second;

		if(!inserted)
		{
			return false;
		}
	}

	if(m_file != (File*)NULL)
	{
		m_file->set_new_index_number(field);

		bool inserted = m_fields_by_index.insert(
			std::unordered_map<int, Field*>::value_type(field->get_number(), field)).second;

		// It shouldn't be possible for that to fail.
		assert(inserted);
		_used_in_assert(inserted);
	}

	m_fields.push_back(field);
	return true;
}
*/

// add_parent adds a new parent to the inheritance hierarchy of the class.
//     Note: This is normally called only during parsing.
void Class::add_parent(Class *parent)
{
	m_parents.push_back(parent);
	m_file->mark_inherited_fields_stale();
}


} // close namespace dclass
