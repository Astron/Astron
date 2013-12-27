// Filename: ArrayParameter.cpp
// Created by: drose (17 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "ArrayParameter.h"
#include "SimpleParameter.h"
#include "ClassParameter.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// basic constructor
ArrayParameter::ArrayParameter(Parameter *element_type, const UnsignedIntRange &size) :
	m_element_type(element_type), m_array_size_range(size)
{
	set_name(m_element_type->get_name());
	m_element_type->set_name(std::string());

	m_array_size = -1;
	if(m_array_size_range.has_one_value())
	{
		m_datatype = DT_array;
		m_array_size = m_array_size_range.get_one_value();
	}
	else
	{
		m_datatype = DT_vararray;
	}


	SimpleParameter *simple_type = m_element_type->as_simple_parameter();
	if(simple_type != (SimpleParameter *)NULL)
	{
		if(simple_type->get_type() == DT_char)
		{
			// We make a special case for char[] arrays: these have the string DataType.
			// Note: A string is equivalent to an int8[] or uint8[] except that it is
			//       treated semantically as character data, and is printed as a string.
			if(m_datatype = DT_array)
			{
				m_datatype = DT_string;
			}
			else
			{
				m_datatype = DT_varstring;
			}
		}
	}

	/*
	if(m_array_size >= 0 && m_element_type->has_fixed_byte_size())
	{
		m_has_fixed_byte_size = true;
		m_fixed_byte_size = m_array_size * m_element_type->get_fixed_byte_size();

	}
	else
	{
		// We only need to store the length bytes if the array has a
		// variable size.
		m_num_length_bytes = sizeof(length_tag_t);
	}

	if(m_element_type->has_range_limits())
	{
		m_has_range_limits = true;
	}

	*/

	if(m_element_type->has_default_value())
	{
		m_has_default_value = true;
	}

	//m_has_nested_fields = true;
	//m_num_nested_fields = m_array_size;
}

// copy constructor
ArrayParameter::ArrayParameter(const ArrayParameter &copy) : Parameter(copy),
	m_element_type(copy.m_element_type->make_copy()),
	m_array_size(copy.m_array_size), m_array_size_range(copy.m_array_size_range)
{
}

// destructor
ArrayParameter::~ArrayParameter()
{
	delete m_element_type;
}

// as_array_parameter returns the same parameter pointer converted to an array parameter
//     pointer, if this is in fact an array parameter; otherwise, returns NULL.
ArrayParameter* ArrayParameter::as_array_parameter()
{
	return this;
}
// as_array_parameter returns the same parameter pointer converted to an array parameter
//     pointer, if this is in fact an array parameter; otherwise, returns NULL.
const ArrayParameter *ArrayParameter::as_array_parameter() const
{
	return this;
}

// make_copy returns a deep copy of this parameter
Parameter* ArrayParameter::make_copy() const
{
	return new ArrayParameter(*this);
}

// is_valid returns false if the element type is an invalid type
//     (e.g. declared from an undefined typedef), or true if it is valid.
bool ArrayParameter::is_valid() const
{
	return m_element_type->is_valid();
}

// get_element_type returns the type of the individual elements of this array.
Parameter* ArrayParameter::get_element_type() const
{
	return m_element_type;
}

// get_array_size returns the fixed number of elements in this array,
//     or -1 if the array may contain a variable number of elements.
int ArrayParameter::get_array_size() const
{
	return m_array_size;
}

// append_array_specification returns the type represented by this type[size].
//     As an ArrayParameter, this same pointer is returned, but the inner type of the array
//     becomes an array type (ie. type[] becomes type[][]).
Parameter* ArrayParameter::append_array_specification(const UnsignedIntRange &size)
{
	if(get_typedef() != (Typedef *)NULL)
	{
		// If this was a typedef, wrap it directly.
		return new ArrayParameter(this, size);
	}

	// Otherwise, the brackets get applied to the inner type.
	m_element_type = m_element_type->append_array_specification(size);
	return this;
}

// output_instance formats the parameter to the syntax of an array parameter in a .dc file
//     as TYPE IDENTIFIER[RANGE] with optional IDENTIFIER and RANGE,
//     and outputs the formatted string to the stream.
void ArrayParameter::output_instance(std::ostream &out, bool brief, const std::string &prename,
                                     const std::string &name, const std::string &postname) const
{
	if(get_typedef() != (Typedef *)NULL)
	{
		output_typedef_name(out, brief, prename, name, postname);

	}
	else
	{
		std::ostringstream strm;

		strm << "[";
		m_array_size_range.output(strm);
		strm << "]";

		m_element_type->output_instance(out, brief, prename, name,
		                                postname + strm.str());
	}
}

// generate_hash accumulates the properties of this type into the hash.
void ArrayParameter::generate_hash(HashGenerator &hashgen) const
{
	Parameter::generate_hash(hashgen);
	m_element_type->generate_hash(hashgen);
	m_array_size_range.generate_hash(hashgen);
}


} // close namespace dclass
