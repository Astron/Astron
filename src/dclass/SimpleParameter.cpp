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
#include "ArrayParameter.h"
#include "ClassParameter.h"
#include "Class.h"
#include "HashGenerator.h"
#include <math.h>
namespace dclass   // open namespace dclass
{


// Type constructor
SimpleParameter::SimpleParameter(DataType type, unsigned int divisor) :
	m_divisor(1), m_has_modulus(false), m_nested_type(DT_invalid), m_bytes_per_element(0)
{
	m_datatype = type;
	//m_has_nested_fields = false;
	//m_num_length_bytes = sizeof(length_tag_t);

	// Check for one of the built-in array types.  For these types, we
	// must present a packing interface that has a variable number of
	// nested fields of the appropriate type.
	switch(type)
	{
		case DT_blob:
			m_nested_type = DT_uint8;
			//m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

		case DT_string:
			m_nested_type = DT_char;
			//m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

			// The simple types can be packed directly.
		case DT_int8:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 1;
			break;

		case DT_int16:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 2;
			break;

		case DT_int32:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 4;
			break;

		case DT_int64:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 8;
			break;

		case DT_char:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 1;
			break;

		case DT_uint8:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 1;
			break;

		case DT_uint16:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 2;
			break;

		case DT_uint32:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 4;
			break;

		case DT_uint64:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 8;
			break;

		case DT_float64:
			//m_has_fixed_byte_size = true;
			//m_fixed_byte_size = 8;
			break;

		case DT_invalid:
			break;
	}

	set_divisor(divisor);

	if(m_nested_type != DT_invalid)
	{
		m_nested_field = create_nested_field(m_nested_type, m_divisor);

	}
	else
	{
		m_nested_field = NULL;
	}
}

// copy constructor
SimpleParameter::SimpleParameter(const SimpleParameter &copy) :
	Parameter(copy), m_divisor(copy.m_divisor),
	m_nested_field(copy.m_nested_field), m_bytes_per_element(copy.m_bytes_per_element),
	m_orig_range(copy.m_orig_range), m_has_modulus(copy.m_has_modulus),
	m_orig_modulus(copy.m_orig_modulus), m_int_range(copy.m_int_range),
	m_uint_range(copy.m_uint_range), m_int64_range(copy.m_int64_range),
	m_uint64_range(copy.m_uint64_range), m_double_range(copy.m_double_range),
	m_uint_modulus(copy.m_uint_modulus), m_uint64_modulus(copy.m_uint64_modulus),
	m_double_modulus(copy.m_double_modulus)
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

// make_copy returns a parameter copy of this parameter
Parameter *SimpleParameter::make_copy() const
{
	return new SimpleParameter(*this);
}

// is_valid returns false if the type is an invalid type (e.g. declared from an undefined typedef).
bool SimpleParameter::is_valid() const
{
	return m_datatype != DT_invalid;
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
int SimpleParameter::get_divisor() const
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

	bool range_error = false;
	m_double_modulus = modulus * m_divisor;
	m_uint64_modulus = (uint64_t)floor(m_double_modulus + 0.5);
	m_uint_modulus = (unsigned int)m_uint64_modulus;

	// Check the range.  The legitimate range for a modulus value is 1
	// through (maximum_value + 1).
/*	switch(m_datatype)
	{
		case DT_int8:
			validate_uint64_limits(m_uint64_modulus - 1, 7, range_error);
			break;

		case DT_int16:
			validate_uint64_limits(m_uint64_modulus - 1, 15, range_error);
			break;

		case DT_int32:
			validate_uint64_limits(m_uint64_modulus - 1, 31, range_error);
			break;

		case DT_int64:
			validate_uint64_limits(m_uint64_modulus - 1, 63, range_error);
			break;

		case DT_char:
		case DT_uint8:
			validate_uint64_limits(m_uint64_modulus - 1, 8, range_error);
			break;

		case DT_uint16:
			validate_uint64_limits(m_uint64_modulus - 1, 16, range_error);
			break;

		case DT_uint32:
			validate_uint64_limits(m_uint64_modulus - 1, 32, range_error);
			break;

		case DT_uint64:
		case DT_float64:
			break;

		default:
			return false;
	}
*/
	return !range_error;
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

// set_range sets the parameter with the indicated range.  A DoubleRange is used for specification,
//     since this is the most generic type; but it is converted to the appropriate type internally.
//     The return value is true if successful, or false if the range is inappropriate for the type.
bool SimpleParameter::set_range(const DoubleRange &range)
{
	bool range_error = false;
	int num_ranges = range.get_num_ranges();
	int i;

	//m_has_range_limits = (num_ranges != 0);
	m_orig_range = range;

	switch(m_datatype)
	{
		case DT_int8:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_int64_limits(min, 8, range_error);
				//validate_int64_limits(max, 8, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case DT_int16:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_int64_limits(min, 16, range_error);
				//validate_int64_limits(max, 16, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case DT_int32:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_int64_limits(min, 32, range_error);
				//validate_int64_limits(max, 32, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case DT_int64:
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				m_int64_range.add_range(min, max);
			}
			break;

		case DT_char:
		case DT_uint8:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_uint64_limits(min, 8, range_error);
				//validate_uint64_limits(max, 8, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case DT_uint16:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_uint64_limits(min, 16, range_error);
				//validate_uint64_limits(max, 16, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case DT_uint32:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_uint64_limits(min, 32, range_error);
				//validate_uint64_limits(max, 32, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case DT_uint64:
			m_uint64_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				m_uint64_range.add_range(min, max);
			}
			break;

		case DT_float64:
			m_double_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				double min = range.get_min(i) * m_divisor;
				double max = range.get_max(i) * m_divisor;
				m_double_range.add_range(min, max);
			}
			break;

		case DT_string:
		case DT_blob:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				//validate_uint64_limits(min, sizeof(length_tag_t) * 8, range_error);
				//validate_uint64_limits(max, sizeof(length_tag_t) * 8, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			if(m_uint_range.has_one_value())
			{
				// If we now have a fixed-length string requirement, we don't
				// need a leading number of bytes.
				//m_num_length_bytes = 0;
				//m_has_fixed_byte_size = true;
				//m_fixed_byte_size = m_uint_range.get_one_value();
			}
			else
			{
				//m_num_length_bytes = sizeof(length_tag_t);
				//m_has_fixed_byte_size = false;
			}
			break;

		default:
			return false;
	}

	return !range_error;
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
		hashgen.add_int((int)m_double_modulus);
	}

	m_int_range.generate_hash(hashgen);
	m_int64_range.generate_hash(hashgen);
	m_uint_range.generate_hash(hashgen);
	m_uint64_range.generate_hash(hashgen);
	m_double_range.generate_hash(hashgen);
}


} // close namespace dclass
