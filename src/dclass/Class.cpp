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
#include "ClassParameter.h"
#include "AtomicField.h"
#include "HashGenerator.h"
#include "File.h"

#include "indent.h"
#include "msgtypes.h"

#include <algorithm>
namespace dclass   // open namespace
{


class SortFieldsByIndex
{
	public:
		inline bool operator()(const Field* a, const Field* b) const
		{
			return a->get_number() < b->get_number();
		}
};

// constructor
Class::Class(File* dc_file, const string &name, bool is_struct, bool bogus_class) :
	m_file(dc_file), m_name(name), m_is_struct(is_struct), m_bogus_class(bogus_class)
{
	m_number = -1;
	m_constructor = NULL;
}

// destructor
Class::~Class()
{
	if(m_constructor != (Field*)NULL)
	{
		delete m_constructor;
	}

	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		delete(*it);
	}
}

// as_class returns the same pointer converted to a class pointer
//     if this is in fact a class; otherwise, returns NULL.
Class* Class::as_class()
{
	return this;
}

// as_class returns the same pointer converted to a class pointer
//     if this is in fact a class; otherwise, returns NULL.
const Class* Class::as_class() const
{
	return this;
}

// get_num_parents returns the number of base classes this class inherits from.
size_t Class::get_num_parents() const
{
	return m_parents.size();
}

// get_parent returns the nth parent class this class inherits from.
Class* Class::get_parent(unsigned int n) const
{
	nassertr(n >= 0 && n < m_parents.size(), NULL);
	return m_parents[n];
}

// has_constructor returns true if this class has a constructor method,
//     or false if it just uses the default constructor.
bool Class::has_constructor() const
{
	return (m_constructor != (Field*)NULL);
}

// get_constructor returns the constructor method for this class if it
//     is defined, or NULL if the class uses the default constructor.
Field* Class::get_constructor() const
{
	return m_constructor;
}

// get_num_fields returns the number of fields defined directly in this class, ignoring inheritance.
size_t Class::get_num_fields() const
{
	return m_fields.size();
}

// get_field returns the nth field in the class.  This is not the field with index n;
//     this is the nth field defined in the class directly, ignoring inheritance.
Field* Class::get_field(unsigned int n) const
{
	nassertr_always(n >= 0 && n < m_fields.size(), NULL);
	return m_fields[n];
}

// get_field_by_name returns a pointer to the declared or inherited Field with name 'name';
//     Returns NULL if there is no such field defined.
Field* Class::get_field_by_name(const string &name) const
{
	auto field_it = m_fields_by_name.find(name);
	if(field_it != m_fields_by_name.end())
	{
		return (*field_it).second;
	}

	// We didn't have such a field, so check our parents.
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		Field* field = (*it)->get_field_by_name(name);
		if(field != (Field*)NULL)
		{
			return field;
		}
	}

	// Nobody knew what this field is.
	return (Field*)NULL;
}

// get_field_by_index returns a pointer to the declared or inherited Field with unique id 'index';
//     Returns NULL if there is no such field defined.
Field *Class::get_field_by_index(int index) const
{
	auto field_it = m_fields_by_index.find(index);
	if(field_it != m_fields_by_index.end())
	{
		return (*field_it).second;
	}

	// We didn't have such a field, so check our parents.
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		Field *field = (*it)->get_field_by_index(index);
		if(field != (Field*)NULL)
		{
			// Cache this result for future lookups.
			// Disabled because should not be caching a value in a const function.
			//m_fields_by_index[index] = field;
			return field;
		}
	}

	// Nobody knew what this field is.
	return (Field*)NULL;
}

// get_field_by_index returns a pointer to the declared or inherited Field with unique id 'index';
//     Returns NULL if there is no such field defined.
Field *Class::get_field_by_index(int index)
{
	auto field_it = m_fields_by_index.find(index);
	if(field_it != m_fields_by_index.end())
	{
		return (*field_it).second;
	}

	// We didn't have such a field, so check our parents.
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		Field *field = (*it)->get_field_by_index(index);
		if(field != (Field*)NULL)
		{
			// Cache this result for future lookups.
			m_fields_by_index[index] = field;
			return field;
		}
	}

	// Nobody knew what this field is.
	return (Field*)NULL;
}

// get_num_inherited_fields returns the total declared and inherited fields in this class.
size_t Class::get_num_inherited_fields()
{
	if(m_file != (File*)NULL)
	{
		m_file->check_inherited_fields();
		if(m_inherited_fields.empty())
		{
			rebuild_inherited_fields();
		}

		// This assertion causes trouble when we are only parsing an incomplete  file.
		//nassertr(is_bogus_class() || !_inherited_fields.empty(), 0);
		return m_inherited_fields.size();
	}
	else
	{
		size_t num_fields = get_num_fields();

		for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
		{
			num_fields += (*it)->get_num_inherited_fields();
		}

		return num_fields;
	}
}

// get_num_inherited_fields returns the total declared and inherited fields in this class.
size_t Class::get_num_inherited_fields() const
{
	if(m_file != (File*)NULL)
	{
		m_file->check_inherited_fields();
		if(m_inherited_fields.empty())
		{
			// Hacky print statement that makes it obvious this needs to be fixed
			std::cerr << "\nTried to get_num_inherited_fields on a possibly uninitialized class with a const class pointer.\n\n";
		}

		// This assertion causes trouble when we are only parsing an incomplete  file.
		//nassertr(is_bogus_class() || !_inherited_fields.empty(), 0);
		return m_inherited_fields.size();
	}
	else
	{
		size_t num_fields = get_num_fields();

		for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
		{
			num_fields += (*it)->get_num_inherited_fields();
		}

		return num_fields;
	}
}

// get_inherited_field returns the nth field from all declared and inherited fields in the class.
Field *Class::get_inherited_field(int n)
{
	if(m_file != (File *)NULL)
	{
		m_file->check_inherited_fields();
		if(m_inherited_fields.empty())
		{
			rebuild_inherited_fields();
		}
		nassertr(n >= 0 && n < (int)m_inherited_fields.size(), NULL);
		return m_inherited_fields[n];
	}
	else
	{
		for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
		{
			size_t num_fields = (*it)->get_num_inherited_fields();
			if(n < num_fields)
			{
				return (*it)->get_inherited_field(n);
			}

			n -= num_fields;
		}

		return get_field(n);
	}
}



// get_inherited_field returns the nth field from all declared and inherited fields in the class.
Field *Class::get_inherited_field(int n) const
{
	if(m_file != (File *)NULL)
	{
		m_file->check_inherited_fields();
		if(m_inherited_fields.empty())
		{
			// Hacky print statement that makes it obvious this needs to be fixed
			std::cerr << "\nTried to get_inherited_field on a possibly uninitialized class with a const class pointer.\n\n";
		}
		nassertr(n >= 0 && n < (int)m_inherited_fields.size(), NULL);
		return m_inherited_fields[n];
	}
	else
	{
		for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
		{
			size_t num_fields = (*it)->get_num_inherited_fields();
			if(n < num_fields)
			{
				return (*it)->get_inherited_field(n);
			}

			n -= num_fields;
		}

		return get_field(n);
	}
}

// inherits_from_bogus_class returns true if this class, or any class in the inheritance
//     heirarchy for this class, is a forward reference to an as-yet-undefined class.
bool Class::inherits_from_bogus_class() const
{
	if(is_bogus_class())
	{
		return true;
	}

	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		if((*it)->inherits_from_bogus_class())
		{
			return true;
		}
	}

	return false;
}

// output formats a string representation of the class in .dc file syntax
//     as "dclass IDENTIFIER" or "struct IDENTIFIER" with structs having optional IDENTIFIER,
//     and outputs the formatted string to the stream.
void Class::output(ostream &out) const
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
void Class::output(ostream &out, bool brief) const
{
	output_instance(out, brief, "", "", "");
}

// write formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Class::write(ostream &out, bool brief, int indent_level) const
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
		if(!(*it)->is_bogus_field())
		{
			(*it)->write(out, brief, indent_level + 2);

			/*
			if (true || (*fi)->has_default_value()) {
			  indent(out, indent_level + 2) << "// = ";
			  Packer packer;
			  packer.set_unpack_data((*fi)->get_default_value());
			  packer.begin_unpack(*fi);
			  packer.unpack_and_format(out, false);
			  if (!packer.end_unpack()) {
			    out << "<error>";
			  }
			  out << "\n";
			}
			*/
		}
	}

	indent(out, indent_level) << "};\n";
}

// output_instance formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Class::output_instance(ostream &out, bool brief, const string &prename,
                            const string &name, const string &postname) const
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
		if(!(*it)->is_bogus_field())
		{
			(*it)->output(out, brief);
			out << "; ";
		}
	}

	out << "}";
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
}

// generate_hash accumulates the properties of this class into the hash.
void Class::generate_hash(HashGenerator &hashgen) const
{
	hashgen.add_string(m_name);

	if(is_struct())
	{
		hashgen.add_int(1);
	}

	hashgen.add_int(m_parents.size());
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		hashgen.add_int((*it)->get_number());
	}

	if(m_constructor != (Field*)NULL)
	{
		m_constructor->generate_hash(hashgen);
	}

	hashgen.add_int(m_fields.size());
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}

// clear_inherited_fields empties the list of inherited fields for the class, so that it
//    may be rebuilt.  This is normally only called by File::rebuild_inherited_fields().
void Class::clear_inherited_fields()
{
	m_inherited_fields.clear();
}

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
				// This local field shadows an inherited field.  Remove the
				// parent's field from our list.
				shadow_inherited_field(field->get_name());
			}

			// Now add the local field.
			m_inherited_fields.push_back(field);
		}
	}

	sort(m_inherited_fields.begin(), m_inherited_fields.end(), SortFieldsByIndex());
}

// shadow_inherited_field this is called only by rebuild_inherited_fields().
//     It removes the named field from the list of m_inherited_fields,
//     presumably in preparation for adding a new definition below.
void Class::shadow_inherited_field(const string &name)
{
	for(auto it = m_inherited_fields.begin(); it != m_inherited_fields.end(); ++it)
	{
		Field *field = (*it);
		if(field->get_name() == name)
		{
			m_inherited_fields.erase(it);
			return;
		}
	}

	// If we get here, the named field wasn't in the list.  Huh.
	nassertv(false);
}

// add_field adds the newly-allocated field to the class.  The class becomes the
//     owner of the pointer and will delete it when it destructs.
//     Returns true if the field is successfully added,
//     or false if there was a name conflict or some other problem.
bool Class::add_field(Field *field)
{
	nassertr(field->get_class() == this || field->get_class() == NULL, false);
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
		nassertr(inserted, false);
		_used_in_assert(inserted);
	}

	m_fields.push_back(field);
	return true;
}

// add_parent adds a new parent to the inheritance hierarchy of the class.
//     Note: This is normally called only during parsing.
void Class::add_parent(Class *parent)
{
	m_parents.push_back(parent);
	m_file->mark_inherited_fields_stale();
}

// set_number assigns the unique number to this class.
//     Note: This is normally called only by the File interface as the class is added.
void Class::set_number(int number)
{
	m_number = number;
}


} // close namespace dclass
