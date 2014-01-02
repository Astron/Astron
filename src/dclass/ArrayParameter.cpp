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
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// basic constructor
ArrayParameter::ArrayParameter(Parameter *element_type, const NumericRange &size) :
	m_element_type(element_type), m_array_range(size)
{
	set_name(m_element_type->get_name());
	m_element_type->set_name(std::string());

	if(!m_array_range.is_empty() && m_array_range.min == m_array_range.max)
	{
		m_datatype = DT_array;
		m_array_size = m_array_range.min.uinteger;
	}
	else
	{
		m_datatype = DT_vararray;
		m_array_size = 0;
	}


	if(m_element_type->get_datatype() == DT_char)
	{
		// We make a special case for char[] arrays: these have the string DataType.
		// Note: A string is equivalent to an int8[] or uint8[] except that it is
		//       treated semantically as character data, and is printed as a string.
		if(m_datatype == DT_array)
		{
			m_datatype = DT_string;
		}
		else
		{
			m_datatype = DT_varstring;
		}
	}

	if(m_array_size > 0 && m_element_type->has_fixed_size())
	{
		m_bytesize = m_array_size * sizeof(m_element_type);
		m_has_fixed_size = true;
	}
	else
	{
		m_bytesize = 0;
		m_has_fixed_size = false;
	}

	/*
	if(m_element_type->has_range_limits())
	{
		m_has_range_limits = true;
	}
	*/

	if(m_element_type->has_default_value())
	{
		m_has_default_value = true;
	}
}

// copy constructor
ArrayParameter::ArrayParameter(const ArrayParameter &copy) : Parameter(copy),
	m_element_type(copy.m_element_type->copy()),
	m_array_size(copy.m_array_size), m_array_range(copy.m_array_range)
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

// copy returns a deep copy of this parameter
Parameter* ArrayParameter::copy() const
{
	return new ArrayParameter(*this);
}

// get_element_type returns the type of the individual elements of this array.
Parameter* ArrayParameter::get_element_type() const
{
	return m_element_type;
}

// get_array_size returns the fixed number of elements in this array,
//     or 0 if the array may contain a variable number of elements.
int ArrayParameter::get_array_size() const
{
	return m_array_size;
}

// append_array_specification returns the type represented by this type[size].
//     As an ArrayParameter, this same pointer is returned, but the inner type of the array
//     becomes an array type (ie. type[] becomes type[][]).
Parameter* ArrayParameter::append_array_specification(const NumericRange &size)
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
		// TODO: fix
		//m_array_size_range.output(strm);
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
	// TODO: fix
	//m_array_size_range.generate_hash(hashgen);
}


} // close namespace dclass
