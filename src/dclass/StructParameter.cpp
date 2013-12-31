// Filename: StructParameter.cpp
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "StructParameter.h"
#include "Class.h"
#include "ArrayParameter.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// construct from class definition
StructParameter::StructParameter(const Struct *dclass) : m_class(dclass)
{
	m_name = dclass->get_name();
	m_datatype = DT_struct;

	int num_fields = 0;
	if(dclass->as_class())
	{
		const Class* cls = dclass->as_class();
		num_fields = cls->get_num_inherited_fields();
		if(cls->has_constructor())
		{
			Field *field = cls->get_constructor();
			num_fields++;
		}
	}
	else
	{
		num_fields = dclass->get_num_fields();
	}



	int i;
	for(i = 0 ; i < num_fields; i++)
	{
		//Field *field = m_class->get_inherited_field(i);
		//if(!field->as_molecular_field())
		//{
			//m_nested_fields.push_back(field);
			//m_has_default_value = m_has_default_value || field->has_default_value();
		//}
	}

	// If all of the nested fields have a fixed byte size, then so does
	// the class (and its byte size is the sum of all of the nested
	// fields).
	/*
	for(i = 0; i < m_num_nested_fields; i++)
	{
		PackerInterface *field = get_nested_field(i);
		m_has_fixed_byte_size = m_has_fixed_byte_size && field->has_fixed_byte_size();
		m_fixed_byte_size += field->get_fixed_byte_size();

		m_has_range_limits = m_has_range_limits || field->has_range_limits();
	}
	*/
}

 // copy constructor
StructParameter::StructParameter(const StructParameter &copy) : Parameter(copy), m_class(copy.m_class)
{
}

// as_class_parameter returns the same parameter pointer converted to a class parameter
//     pointer, if this is in fact an class parameter; otherwise, returns NULL.
StructParameter *StructParameter::as_struct_parameter()
{
	return this;
}


// as_class_parameter returns the same parameter pointer converted to a class parameter
//     pointer, if this is in fact an class parameter; otherwise, returns NULL.
const StructParameter *StructParameter::as_struct_parameter() const
{
	return this;
}

// make_copy returns a deep copy of this parameter
Parameter *StructParameter::make_copy() const
{
	return new StructParameter(*this);
}

// is_valid returns false if the element type is an invalid type
//     (e.g. declared from an undefined typedef), or true if it is valid.
bool StructParameter::is_valid() const
{
	return true;
}

// get_class returns the class that this parameter represents
const Struct *StructParameter::get_class() const
{
	return m_class;
}


// output_instance formats the parameter to the syntax of an class parameter in a .dc file
//     as CLASS_IDENTIFIER PARAM_IDENTIFIER with optional PARAM_IDENTIFIER,
//     and outputs the formatted string to the stream.
void StructParameter::output_instance(std::ostream &out, bool brief, const std::string &prename,
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
void StructParameter::generate_hash(HashGenerator &hashgen) const
{
	Parameter::generate_hash(hashgen);
	m_class->generate_hash(hashgen);
}



} // close namespace dclass
