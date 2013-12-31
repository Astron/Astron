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
using namespace std;
namespace dclass   // open namespace
{


// construct from class definition
StructParameter::StructParameter(const Struct *dclass) : Struct(dclass), m_class(dclass)
{
	m_name = dclass->get_name();
	m_datatype = DT_struct;

	size_t num_fields = dclass->get_num_fields();
	for(unsigned int i = 0 ; i < num_fields; i++)
	{
		Field *field = dclass->get_field(i);
		if(!field->as_molecular_field()) // Field is Atomic or Parameter
		{
			m_fields.push_back(field);
		}
	}

	// If all of the nested fields have a fixed byte size, then so does the class
	// (and its byte size is the sum of all of its fields).
	for(unsigned int i = 0; i < m_fields.size(); i++)
	{
		Field *field = m_fields[i];
		if(!field->has_fixed_size())
		{
			m_has_fixed_size = false;
			m_bytesize = 0;
			break;
		}

		m_bytesize += field->get_size();
	}
}

 // copy constructor
StructParameter::StructParameter(const StructParameter &copy) :
	Struct(copy), Parameter(copy), m_class(copy.m_class)
{
	m_file = copy.m_file;
	m_name = string(copy.m_name);
	m_fields = vector<Field*>(copy.m_fields);
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

// copy returns a deep copy of this parameter
Parameter *StructParameter::copy() const
{
	return new StructParameter(*this);
}

// get_class returns the class that this parameter represents
const Struct *StructParameter::get_class() const
{
	return m_class;
}

// output_instance formats the parameter to the syntax of an class parameter in a .dc file
//     as CLASS_IDENTIFIER PARAM_IDENTIFIER with optional PARAM_IDENTIFIER,
//     and outputs the formatted string to the stream.
void StructParameter::output_instance(ostream &out, bool brief, const string &prename,
                                     const string &name, const string &postname) const
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

bool add_field(Field* field)
{
	return false;
}



} // close namespace dclass
