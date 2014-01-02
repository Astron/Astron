// Filename: SimpleParameter.cpp
// Created by: drose (15 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "SimpleParameter.h"
#include "Typedef.h"
#include "Class.h"
#include "HashGenerator.h"
#include <math.h>
namespace dclass   // open namespace dclass
{


// Type constructor
SimpleParameter::SimpleParameter(DataType type, unsigned int divisor) :
	m_divisor(divisor), m_has_modulus(false)
{
	m_datatype = type;
	switch(type)
	{
		// The simple types can be packed directly.
		case DT_int8:
			m_bytesize = sizeof(int8_t);
			break;
		case DT_int16:
			m_bytesize = sizeof(int16_t);
			break;
		case DT_int32:
			m_bytesize = sizeof(int32_t);
			break;
		case DT_int64:
			m_bytesize = sizeof(int64_t);
			break;
		case DT_char:
			m_bytesize = sizeof(char);
			break;
		case DT_uint8:
			m_bytesize = sizeof(uint8_t);
			break;
		case DT_uint16:
			m_bytesize = sizeof(uint16_t);
			break;
		case DT_uint32:
			m_bytesize = sizeof(uint32_t);
			break;
		case DT_uint64:
			m_bytesize = sizeof(uint64_t);
			break;
		case DT_float32:
			m_bytesize = sizeof(float);
			break;
		case DT_float64:
			m_bytesize = sizeof(double);
			break;
		case DT_string:
			// Strings are variable by default, they become static
			// only after setting a fixed-width range.
			m_datatype = DT_varstring;
			m_has_fixed_size = false;
		case DT_blob:
			// Blobs are variable by default, they become static
			// only after setting a fixed-width range.
			m_datatype = DT_varblob;
			m_has_fixed_size = false;
		case DT_invalid:
			break;
		default:
			m_datatype = DT_invalid;
	}
}

// copy constructor
SimpleParameter::SimpleParameter(const SimpleParameter &copy) :
	Parameter(copy), m_divisor(copy.m_divisor), m_has_modulus(copy.m_has_modulus),
	m_orig_modulus(copy.m_orig_modulus), m_orig_range(copy.m_orig_range),
	m_modulus(copy.m_modulus), m_range(copy.m_range)
{
	m_datatype = copy.m_datatype;
}

// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
//     if this is in fact a simple parameter; otherwise, returns NULL.
SimpleParameter *SimpleParameter::as_simple_parameter()
{
	return this;
}

// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
//     if this is in fact a simple parameter; otherwise, returns NULL.
const SimpleParameter *SimpleParameter::as_simple_parameter() const
{
	return this;
}

// copy returns a parameter copy of this parameter
Parameter* SimpleParameter::copy() const
{
	return new SimpleParameter(*this);
}

// has_modulus returns true if there is a modulus associated, false otherwise.
bool SimpleParameter::has_modulus() const
{
	return m_has_modulus;
}

// get_modulus returns the modulus associated with this type, if any.
//     It is an error to call this if has_modulus() returned false.
//     If present, this is the modulus that is used to constrain the numeric value of
//     the field before it is packed (and range-checked).
double SimpleParameter::get_modulus() const
{
	return m_orig_modulus;
}

// get_divisor returns the divisor associated with this type.  This is 1 by default,
//     but if this is other than one it represents the scale to apply when packing and
//     unpacking numeric values (to store fixed-point values in an integer field).
//     It is only meaningful for numeric-type fields.
unsigned int SimpleParameter::get_divisor() const
{
	return m_divisor;
}

// is_numeric_type returns true if the type is a numeric type (and therefore can accept a divisor
//     and/or a modulus), or false if it is some string-based type.
bool SimpleParameter::is_numeric_type() const
{
	return m_datatype <= DT_float64;
}

// set_modulus assigns the indicated modulus to the simple type.
//     Any packed value will be constrained to be within [0, modulus).
//     Returns true if assigned, false if this type cannot accept a modulus or if the modulus is invalid.
bool SimpleParameter::set_modulus(double modulus)
{
	if(!is_numeric_type() || modulus <= 0.0)
	{
		return false;
	}

	m_has_modulus = true;
	m_orig_modulus = modulus;

	switch(m_datatype)
	{
		case DT_uint8:
		case DT_uint16:
		case DT_uint32:
		case DT_uint64:
		case DT_int8:
		case DT_int16:
		case DT_int32:
		case DT_int64:
			m_modulus = Number((unsigned int)floor(modulus * m_divisor + 0.5));
		case DT_float32:
		case DT_float64:
			m_modulus = Number(modulus * m_divisor);
		default:
			return false;
	}

	return true;

	// TODO:
	// Check the range.  The legitimate range for a modulus value is 1
	// through (maximum_value + 1).
}

// set_divisor assigns the indicated divisor to the simple type.
//     Returns true if assigned, false if this type cannot accept a divisor or if the divisor is invalid.
bool SimpleParameter::set_divisor(unsigned int divisor)
{
	if(!is_numeric_type() || divisor == 0)
	{
		return false;
	}

	m_divisor = divisor;
	if((m_divisor != 1) && (m_datatype <= DT_uint64)) // am I an integer-numeric type
	{
		m_datatype = DT_float64;
	}

	/* TODO */
	//if(m_has_range_limits)
	//{
	//	set_range(m_orig_range);
	//}
	if(m_has_modulus)
	{
		set_modulus(m_orig_modulus);
	}

	return true;
}

// set_range sets the parameter with the indicated range.
//     It is currently assumed the range type is NT_float, TODO: fix.
//     The return value is true if successful, or false if the range is inappropriate for the type.
bool SimpleParameter::set_range(const NumericRange &range)
{
	m_orig_range = range;
	switch(m_datatype)
	{
		case DT_int8:
		case DT_int16:
		case DT_int32:
		case DT_int64:
		{
			int64_t min = (int64_t)floor(range.min.floating * m_divisor + 0.5);
			int64_t max = (int64_t)floor(range.max.floating * m_divisor + 0.5);
			m_range = NumericRange(min, max);
			// TODO: Validate range
			break;
		}
		case DT_char:
		case DT_uint8:
		case DT_uint16:
		case DT_uint32:
		case DT_uint64:
		{
			uint64_t min = (uint64_t)floor(range.min.floating * m_divisor + 0.5);
			uint64_t max = (uint64_t)floor(range.max.floating * m_divisor + 0.5);
			m_range = NumericRange(min, max);
			// TODO: Validate range
			break;
		}
		case DT_float32:
		case DT_float64:
		{
			double min = range.min.floating * m_divisor;
			double max = range.max.floating * m_divisor;
			m_range = NumericRange(min, max);
			break;
		}
		case DT_string:
		case DT_varstring:
		{
			uint64_t min = (uint64_t)floor(range.min.floating * m_divisor + 0.5);
			uint64_t max = (uint64_t)floor(range.max.floating * m_divisor + 0.5);
			m_range = NumericRange(min, max);
			// TODO: Validate range
			if(m_range.min == m_range.max)
			{
				m_datatype = DT_string;
				m_has_fixed_size = true;
			}
			else
			{
				m_datatype = DT_varstring;
				m_has_fixed_size = false;
			}
			break;
		}
		case DT_blob:
		case DT_varblob:
		{
			uint64_t min = (uint64_t)floor(range.min.floating * m_divisor + 0.5);
			uint64_t max = (uint64_t)floor(range.max.floating * m_divisor + 0.5);
			m_range = NumericRange(min, max);
			// TODO: Validate range
			if(m_range.min == m_range.max)
			{
				m_datatype = DT_blob;
				m_has_fixed_size = true;
			}
			else
			{
				m_datatype = DT_varblob;
				m_has_fixed_size = false;
			}
			break;
		}
		default:
			return false;
	}

	return true;
}

// output_instance formats the parameter in .dc syntax as a typename and identifier.
void SimpleParameter::output_instance(std::ostream &out, bool brief, const std::string &prename,
                                      const std::string &name, const std::string &postname) const
{
	if(get_typedef() != (Typedef *)NULL)
	{
		output_typedef_name(out, brief, prename, name, postname);

	}
	else
	{
		out << m_datatype;
		if(m_has_modulus)
		{
			out << "%" << m_orig_modulus;
		}
		if(m_divisor != 1)
		{
			out << "/" << m_divisor;
		}

		// TODO: Replace
		/*
		switch(m_datatype)
		{
			case DT_int8:
			case DT_int16:
			case DT_int32:
				if(!m_int_range.is_empty())
				{
					out << "(";
					m_int_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case DT_int64:
				if(!m_int64_range.is_empty())
				{
					out << "(";
					m_int64_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case DT_uint8:
			case DT_uint16:
			case DT_uint32:
				if(!m_uint_range.is_empty())
				{
					out << "(";
					m_uint_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case DT_char:
				if(!m_uint_range.is_empty())
				{
					out << "(";
					m_uint_range.output_char(out, m_divisor);
					out << ")";
				}
				break;

			case DT_uint64:
				if(!m_uint64_range.is_empty())
				{
					out << "(";
					m_uint64_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case DT_float64:
				if(!m_double_range.is_empty())
				{
					out << "(";
					m_double_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case DT_string:
				if(!m_uint_range.is_empty())
				{
					out << "(";
					m_uint_range.output(out, m_divisor);
					out << ")";
				}
				break;
			default:
				break;
		}
		*/

		if(!prename.empty() || !name.empty() || !postname.empty())
		{
			out << " " << prename << name << postname;
		}
	}
}

// generate_hash accumulates the properties of this type into the hash.
void SimpleParameter::generate_hash(HashGenerator &hashgen) const
{
	Parameter::generate_hash(hashgen);

	hashgen.add_int(m_datatype);
	hashgen.add_int(m_divisor);
	if(m_has_modulus)
	{
		hashgen.add_int(m_modulus.sinteger);
	}

	hashgen.add_int(m_range.min.sinteger);
	hashgen.add_int(m_range.max.sinteger);
	//m_int_range.generate_hash(hashgen);
	//m_int64_range.generate_hash(hashgen);
	//m_uint_range.generate_hash(hashgen);
	//m_uint64_range.generate_hash(hashgen);
	//m_double_range.generate_hash(hashgen);
}


} // close namespace dclass
