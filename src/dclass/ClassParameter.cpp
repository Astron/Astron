// Filename: ClassParameter.cpp
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "ClassParameter.h"
#include "Class.h"
#include "ArrayParameter.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// construct from class definition
ClassParameter::ClassParameter(const Class *dclass) : m_class(dclass)
{
	set_name(dclass->get_name());

	int num_fields = m_class->get_num_inherited_fields();

	m_has_nested_fields = true;
	m_pack_type = PT_class;

	if(m_class->has_constructor())
	{
		Field *field = m_class->get_constructor();
		m_nested_fields.push_back(field);
		m_has_default_value = m_has_default_value || field->has_default_value();
	}
	int i;
	for(i = 0 ; i < num_fields; i++)
	{
		Field *field = m_class->get_inherited_field(i);
		if(!field->as_molecular_field())
		{
			m_nested_fields.push_back(field);
			m_has_default_value = m_has_default_value || field->has_default_value();
		}
	}
	m_num_nested_fields = m_nested_fields.size();

	// If all of the nested fields have a fixed byte size, then so does
	// the class (and its byte size is the sum of all of the nested
	// fields).
	m_has_fixed_byte_size = true;
	m_fixed_byte_size = 0;
	for(i = 0; i < m_num_nested_fields; i++)
	{
		PackerInterface *field = get_nested_field(i);
		m_has_fixed_byte_size = m_has_fixed_byte_size && field->has_fixed_byte_size();
		m_fixed_byte_size += field->get_fixed_byte_size();

		m_has_range_limits = m_has_range_limits || field->has_range_limits();
	}
}

 // copy constructor
ClassParameter::ClassParameter(const ClassParameter &copy) :
	Parameter(copy), m_class(copy.m_class), m_nested_fields(copy.m_nested_fields)
{
}

// as_class_parameter returns the same parameter pointer converted to a class parameter
//     pointer, if this is in fact an class parameter; otherwise, returns NULL.
ClassParameter *ClassParameter::as_class_parameter()
{
	return this;
}


// as_class_parameter returns the same parameter pointer converted to a class parameter
//     pointer, if this is in fact an class parameter; otherwise, returns NULL.
const ClassParameter *ClassParameter::as_class_parameter() const
{
	return this;
}

// make_copy returns a deep copy of this parameter
Parameter *ClassParameter::make_copy() const
{
	return new ClassParameter(*this);
}

// is_valid returns false if the element type is an invalid type
//     (e.g. declared from an undefined typedef), or true if it is valid.
bool ClassParameter::is_valid() const
{
	return !m_class->is_bogus_class();
}

// get_class returns the class that this parameter represents
const Class *ClassParameter::get_class() const
{
	return m_class;
}


// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     The return is NULL if 'n' is out-of-bounds of 0 <= n < get_num_nested_fields().
PackerInterface *ClassParameter::get_nested_field(int n) const
{
	assert(n >= 0 && n < (int)m_nested_fields.size());
	return m_nested_fields[n];
}

// output_instance formats the parameter to the syntax of an class parameter in a .dc file
//     as CLASS_IDENTIFIER PARAM_IDENTIFIER with optional PARAM_IDENTIFIER,
//     and outputs the formatted string to the stream.
void ClassParameter::output_instance(std::ostream &out, bool brief, const std::string &prename,
                                     const std::string &name, const std::string &postname) const
{
	if(get_typedef() != (Typedef*)NULL)
	{
		output_typedef_name(out, brief, prename, name, postname);

	}
	else
	{
		m_class->output_instance(out, brief, prename, name, postname);
	}
}

// generate_hash accumulates the properties of this type into the hash.
void ClassParameter::generate_hash(HashGenerator &hashgen) const
{
	Parameter::generate_hash(hashgen);
	m_class->generate_hash(hashgen);
}

// do_check_match returns true if the other interface is bitwise the same as
//     this one--that is, a uint32 only matches a uint32, etc.
//     Names of components, and range limits, are not compared.
bool ClassParameter::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_class_parameter(this);
}

// do_check_match_class_parameter returns true if this field matches the
//     indicated class parameter, or false otherwise.
bool ClassParameter::do_check_match_class_parameter(const ClassParameter *other) const
{
	if(m_nested_fields.size() != other->m_nested_fields.size())
	{
		return false;
	}
	for(size_t i = 0; i < m_nested_fields.size(); i++)
	{
		if(!m_nested_fields[i]->check_match(other->m_nested_fields[i]))
		{
			return false;
		}
	}

	return true;
}

// do_check_match_array_parameter returns true if this field matches the
//    indicated array parameter, or false otherwise.
bool ClassParameter::do_check_match_array_parameter(const ArrayParameter *other) const
{
	if((int)m_nested_fields.size() != other->get_array_size())
	{
		// We can only match a fixed-size array whose size happens to
		// exactly match our number of fields.
		return false;
	}

	const PackerInterface *element_type = other->get_element_type();
	for(size_t i = 0; i < m_nested_fields.size(); i++)
	{
		if(!m_nested_fields[i]->check_match(element_type))
		{
			return false;
		}
	}

	return true;
}


} // close namespace dclass
