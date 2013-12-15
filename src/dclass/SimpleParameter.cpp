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
#include "PackData.h"
#include "Typedef.h"
#include "ArrayParameter.h"
#include "ClassParameter.h"
#include "Class.h"
#include "HashGenerator.h"
#include <math.h>
namespace dclass   // open namespace dclass
{



std::map<SubatomicType, std::map<unsigned int, SimpleParameter*> > SimpleParameter::cls_nested_field_map;
ClassParameter *SimpleParameter::cls_uint32uint8_type = NULL;

// Type constructor
SimpleParameter::SimpleParameter(SubatomicType type, unsigned int divisor) :
	m_type(type), m_divisor(1), m_has_modulus(false), m_nested_type(ST_invalid), m_bytes_per_element(0)
{
	m_pack_type = PT_invalid;
	m_has_nested_fields = false;	
	m_num_length_bytes = sizeof(length_tag_t);
	// Check for one of the built-in array types.  For these types, we
	// must present a packing interface that has a variable number of
	// nested fields of the appropriate type.
	switch(m_type)
	{
		case ST_int8array:
			m_pack_type = PT_array;
			m_nested_type = ST_int8;
			m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

		case ST_int16array:
			m_pack_type = PT_array;
			m_nested_type = ST_int16;
			m_has_nested_fields = true;
			m_bytes_per_element = 2;
			break;

		case ST_int32array:
			m_pack_type = PT_array;
			m_nested_type = ST_int32;
			m_has_nested_fields = true;
			m_bytes_per_element = 4;
			break;

		case ST_uint8array:
			m_pack_type = PT_array;
			m_nested_type = ST_uint8;
			m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

		case ST_uint16array:
			m_pack_type = PT_array;
			m_nested_type = ST_uint16;
			m_has_nested_fields = true;
			m_bytes_per_element = 2;
			break;

		case ST_uint32array:
			m_pack_type = PT_array;
			m_nested_type = ST_uint32;
			m_has_nested_fields = true;
			m_bytes_per_element = 4;
			break;

		case ST_uint32uint8array:
			m_pack_type = PT_array;
			m_has_nested_fields = true;
			m_bytes_per_element = 5;
			break;

		case ST_blob32:
			m_num_length_bytes = 4;
			// fall through
		case ST_blob:
			// For blob and string, we will present an array interface
			// as an array of uint8, but we will also accept a set_value()
			// with a string parameter.
			m_pack_type = PT_blob;
			m_nested_type = ST_uint8;
			m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

		case ST_string:
			m_pack_type = PT_string;
			m_nested_type = ST_char;
			m_has_nested_fields = true;
			m_bytes_per_element = 1;
			break;

			// The simple types can be packed directly.
		case ST_int8:
			m_pack_type = PT_int;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 1;
			break;

		case ST_int16:
			m_pack_type = PT_int;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 2;
			break;

		case ST_int32:
			m_pack_type = PT_int;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 4;
			break;

		case ST_int64:
			m_pack_type = PT_int64;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 8;
			break;

		case ST_char:
			m_pack_type = PT_string;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 1;
			break;

		case ST_uint8:
			m_pack_type = PT_uint;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 1;
			break;

		case ST_uint16:
			m_pack_type = PT_uint;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 2;
			break;

		case ST_uint32:
			m_pack_type = PT_uint;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 4;
			break;

		case ST_uint64:
			m_pack_type = PT_uint64;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 8;
			break;

		case ST_float64:
			m_pack_type = PT_double;
			m_has_fixed_byte_size = true;
			m_fixed_byte_size = 8;
			break;

		case ST_invalid:
			break;
	}

	set_divisor(divisor);

	if(m_nested_type != ST_invalid)
	{
		m_nested_field = create_nested_field(m_nested_type, m_divisor);

	}
	else if(m_type == ST_uint32uint8array)
	{
		// This one is a special case.  We must create a special nested
		// type that accepts a uint32 followed by a uint8 for each
		// element.
		m_nested_field = create_uint32uint8_type();

	}
	else
	{
		m_nested_field = NULL;
	}
}

// copy constructor
SimpleParameter::SimpleParameter(const SimpleParameter &copy) :
	Parameter(copy), m_type(copy.m_type), m_divisor(copy.m_divisor),
	m_nested_field(copy.m_nested_field), m_bytes_per_element(copy.m_bytes_per_element),
	m_orig_range(copy.m_orig_range), m_has_modulus(copy.m_has_modulus),
	m_orig_modulus(copy.m_orig_modulus), m_int_range(copy.m_int_range),
	m_uint_range(copy.m_uint_range), m_int64_range(copy.m_int64_range),
	m_uint64_range(copy.m_uint64_range), m_double_range(copy.m_double_range),
	m_uint_modulus(copy.m_uint_modulus), m_uint64_modulus(copy.m_uint64_modulus),
	m_double_modulus(copy.m_double_modulus)
{
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
	return m_type != ST_invalid;
}

// get_type returns the particular subatomic type represented by this instance.
SubatomicType SimpleParameter::get_type() const
{
	return m_type;
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
	return !(m_pack_type == PT_string || m_pack_type == PT_blob);
}

// set_modulus assigns the indicated modulus to the simple type.
//     Any packed value will be constrained to be within [0, modulus).
//     Returns true if assigned, false if this type cannot accept a modulus or if the modulus is invalid.
bool SimpleParameter::set_modulus(double modulus)
{
	if(m_pack_type == PT_string || m_pack_type == PT_blob || modulus <= 0.0)
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
	switch(m_type)
	{
		case ST_int8:
		case ST_int8array:
			validate_uint64_limits(m_uint64_modulus - 1, 7, range_error);
			break;

		case ST_int16:
		case ST_int16array:
			validate_uint64_limits(m_uint64_modulus - 1, 15, range_error);
			break;

		case ST_int32:
		case ST_int32array:
			validate_uint64_limits(m_uint64_modulus - 1, 31, range_error);
			break;

		case ST_int64:
			validate_uint64_limits(m_uint64_modulus - 1, 63, range_error);
			break;

		case ST_char:
		case ST_uint8:
		case ST_uint8array:
			validate_uint64_limits(m_uint64_modulus - 1, 8, range_error);
			break;

		case ST_uint16:
		case ST_uint16array:
			validate_uint64_limits(m_uint64_modulus - 1, 16, range_error);
			break;

		case ST_uint32:
		case ST_uint32array:
			validate_uint64_limits(m_uint64_modulus - 1, 32, range_error);
			break;

		case ST_uint64:
		case ST_float64:
			break;

		default:
			return false;
	}

	return !range_error;
}

// set_divisor assigns the indicated divisor to the simple type.
//     Returns true if assigned, false if this type cannot accept a divisor or if the divisor is invalid.
bool SimpleParameter::set_divisor(unsigned int divisor)
{
	if(m_pack_type == PT_string || m_pack_type == PT_blob || divisor == 0)
	{
		return false;
	}

	m_divisor = divisor;
	if((m_divisor != 1) &&
	        (m_pack_type == PT_int || m_pack_type == PT_int64 ||
	         m_pack_type == PT_uint || m_pack_type == PT_uint64))
	{
		m_pack_type = PT_double;
	}

	if(m_has_range_limits)
	{
		set_range(m_orig_range);
	}
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

	m_has_range_limits = (num_ranges != 0);
	m_orig_range = range;

	switch(m_type)
	{
		case ST_int8:
		case ST_int8array:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_int64_limits(min, 8, range_error);
				validate_int64_limits(max, 8, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case ST_int16:
		case ST_int16array:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_int64_limits(min, 16, range_error);
				validate_int64_limits(max, 16, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case ST_int32:
		case ST_int32array:
			m_int_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_int64_limits(min, 32, range_error);
				validate_int64_limits(max, 32, range_error);
				m_int_range.add_range((int)min, (int)max);
			}
			break;

		case ST_int64:
			m_int64_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				int64_t min = (int64_t)floor(range.get_min(i) * m_divisor + 0.5);
				int64_t max = (int64_t)floor(range.get_max(i) * m_divisor + 0.5);
				m_int64_range.add_range(min, max);
			}
			break;

		case ST_char:
		case ST_uint8:
		case ST_uint8array:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_uint64_limits(min, 8, range_error);
				validate_uint64_limits(max, 8, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case ST_uint16:
		case ST_uint16array:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_uint64_limits(min, 16, range_error);
				validate_uint64_limits(max, 16, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case ST_uint32:
		case ST_uint32array:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_uint64_limits(min, 32, range_error);
				validate_uint64_limits(max, 32, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			break;

		case ST_uint64:
			m_uint64_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				m_uint64_range.add_range(min, max);
			}
			break;

		case ST_float64:
			m_double_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				double min = range.get_min(i) * m_divisor;
				double max = range.get_max(i) * m_divisor;
				m_double_range.add_range(min, max);
			}
			break;

		case ST_string:
		case ST_blob:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_uint64_limits(min, sizeof(length_tag_t) * 8, range_error);
				validate_uint64_limits(max, sizeof(length_tag_t) * 8, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			if(m_uint_range.has_one_value())
			{
				// If we now have a fixed-length string requirement, we don't
				// need a leading number of bytes.
				m_num_length_bytes = 0;
				m_has_fixed_byte_size = true;
				m_fixed_byte_size = m_uint_range.get_one_value();
			}
			else
			{
				m_num_length_bytes = sizeof(length_tag_t);
				m_has_fixed_byte_size = false;
			}
			break;

		case ST_blob32:
			m_uint_range.clear();
			for(i = 0; i < num_ranges; i++)
			{
				uint64_t min = (uint64_t)floor(range.get_min(i) * m_divisor + 0.5);
				uint64_t max = (uint64_t)floor(range.get_max(i) * m_divisor + 0.5);
				validate_uint64_limits(min, 32, range_error);
				validate_uint64_limits(max, 32, range_error);
				m_uint_range.add_range((unsigned int)min, (unsigned int)max);
			}
			if(m_uint_range.has_one_value())
			{
				// If we now have a fixed-length string requirement, we don't
				// need a leading number of bytes.
				m_num_length_bytes = 0;
				m_has_fixed_byte_size = true;
				m_fixed_byte_size = m_uint_range.get_one_value();
			}
			else
			{
				m_num_length_bytes = 4;
				m_has_fixed_byte_size = false;
			}
			break;

		default:
			return false;
	}

	return !range_error;
}

// calc_num_nested_fields returns the number of nested fields to expect, given a certain length
//     in bytes (as read from the m_num_length_bytes stored in the stream on the push).
//     This will only be called if m_num_length_bytes is nonzero.
int SimpleParameter::calc_num_nested_fields(size_t length_bytes) const
{
	if(m_bytes_per_element != 0)
	{
		return length_bytes / m_bytes_per_element;
	}
	return 0;
}

// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     This may return NULL if there n is outside of the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
PackerInterface *SimpleParameter::get_nested_field(int) const
{
	return m_nested_field;
}

// pack_double packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_double(PackData &pack_data, double value,
                                  bool &pack_error, bool &range_error) const
{
	double real_value = value * m_divisor;
	if(m_has_modulus)
	{
		if(real_value < 0.0)
		{
			real_value = m_double_modulus - fmod(-real_value, m_double_modulus);
			if(real_value == m_double_modulus)
			{
				real_value = 0.0;
			}
		}
		else
		{
			real_value = fmod(real_value, m_double_modulus);
		}
	}

	switch(m_type)
	{
		case ST_int8:
		{
			int int_value = (int)floor(real_value + 0.5);
			m_int_range.validate(int_value, range_error);
			validate_int_limits(int_value, 8, range_error);
			do_pack_int8(pack_data.get_write_pointer(1), int_value);
		}
		break;

		case ST_int16:
		{
			int int_value = (int)floor(real_value + 0.5);
			m_int_range.validate(int_value, range_error);
			validate_int_limits(int_value, 16, range_error);
			do_pack_int16(pack_data.get_write_pointer(2), int_value);
		}
		break;

		case ST_int32:
		{
			int int_value = (int)floor(real_value + 0.5);
			m_int_range.validate(int_value, range_error);
			do_pack_int32(pack_data.get_write_pointer(4), int_value);
		}
		break;

		case ST_int64:
		{
			int64_t int64_value = (int64_t)floor(real_value + 0.5);
			m_int64_range.validate(int64_value, range_error);
			do_pack_int64(pack_data.get_write_pointer(8), int64_value);
		}
		break;

		case ST_char:
		case ST_uint8:
		{
			unsigned int int_value = (unsigned int)floor(real_value + 0.5);
			m_uint_range.validate(int_value, range_error);
			validate_uint_limits(int_value, 8, range_error);
			do_pack_uint8(pack_data.get_write_pointer(1), int_value);
		}
		break;

		case ST_uint16:
		{
			unsigned int int_value = (unsigned int)floor(real_value + 0.5);
			m_uint_range.validate(int_value, range_error);
			validate_uint_limits(int_value, 16, range_error);
			do_pack_uint16(pack_data.get_write_pointer(2), int_value);
		}
		break;

		case ST_uint32:
		{
			unsigned int int_value = (unsigned int)floor(real_value + 0.5);
			m_uint_range.validate(int_value, range_error);
			do_pack_uint32(pack_data.get_write_pointer(4), int_value);
		}
		break;

		case ST_uint64:
		{
			uint64_t int64_value = (uint64_t)floor(real_value + 0.5);
			m_uint64_range.validate(int64_value, range_error);
			do_pack_uint64(pack_data.get_write_pointer(8), int64_value);
		}
		break;

		case ST_float64:
			m_double_range.validate(real_value, range_error);
			do_pack_float64(pack_data.get_write_pointer(8), real_value);
			break;

		default:
			pack_error = true;
	}
}

// pack_int packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_int(PackData &pack_data, int value,
                               bool &pack_error, bool &range_error) const
{
	int int_value = value * m_divisor;

	if(value != 0 && (int_value / value) != (int)m_divisor)
	{
		// If we've experienced overflow after applying the divisor, pack
		// it as an int64 instead.
		pack_int64(pack_data, (int64_t)value, pack_error, range_error);
		return;
	}

	if(m_has_modulus && m_uint_modulus != 0)
	{
		if(int_value < 0)
		{
			int_value = m_uint_modulus - 1 - (-int_value - 1) % m_uint_modulus;
		}
		else
		{
			int_value = int_value % m_uint_modulus;
		}
	}

	switch(m_type)
	{
		case ST_int8:
			m_int_range.validate(int_value, range_error);
			validate_int_limits(int_value, 8, range_error);
			do_pack_int8(pack_data.get_write_pointer(1), int_value);
			break;

		case ST_int16:
			m_int_range.validate(int_value, range_error);
			validate_int_limits(int_value, 16, range_error);
			do_pack_int16(pack_data.get_write_pointer(2), int_value);
			break;

		case ST_int32:
			m_int_range.validate(int_value, range_error);
			do_pack_int32(pack_data.get_write_pointer(4), int_value);
			break;

		case ST_int64:
			m_int64_range.validate(int_value, range_error);
			do_pack_int64(pack_data.get_write_pointer(8), int_value);
			break;

		case ST_char:
		case ST_uint8:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)int_value, range_error);
			validate_uint_limits((unsigned int)int_value, 8, range_error);
			do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)int_value);
			break;

		case ST_uint16:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)int_value, range_error);
			validate_uint_limits((unsigned int)int_value, 16, range_error);
			do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)int_value);
			break;

		case ST_uint32:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)int_value, range_error);
			do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)int_value);
			break;

		case ST_uint64:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint64_range.validate((unsigned int)int_value, range_error);
			do_pack_uint64(pack_data.get_write_pointer(8), (unsigned int)int_value);
			break;

		case ST_float64:
			m_double_range.validate(int_value, range_error);
			do_pack_float64(pack_data.get_write_pointer(8), int_value);
			break;

		default:
			pack_error = true;
	}
}

// pack_uint packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_uint(PackData &pack_data, unsigned int value,
                                bool &pack_error, bool &range_error) const
{
	unsigned int int_value = value * m_divisor;
	if(m_has_modulus && m_uint_modulus != 0)
	{
		int_value = int_value % m_uint_modulus;
	}

	switch(m_type)
	{
		case ST_int8:
			if((int)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)int_value, range_error);
			validate_int_limits((int)int_value, 8, range_error);
			do_pack_int8(pack_data.get_write_pointer(1), (int)int_value);
			break;

		case ST_int16:
			if((int)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)int_value, range_error);
			validate_int_limits((int)int_value, 16, range_error);
			do_pack_int16(pack_data.get_write_pointer(2), (int)int_value);
			break;

		case ST_int32:
			if((int)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)int_value, range_error);
			do_pack_int32(pack_data.get_write_pointer(4), (int)int_value);
			break;

		case ST_int64:
			if((int)int_value < 0)
			{
				range_error = true;
			}
			m_int64_range.validate((int)int_value, range_error);
			do_pack_int64(pack_data.get_write_pointer(8), (int)int_value);
			break;

		case ST_char:
		case ST_uint8:
			m_uint_range.validate(int_value, range_error);
			validate_uint_limits(int_value, 8, range_error);
			do_pack_uint8(pack_data.get_write_pointer(1), int_value);
			break;

		case ST_uint16:
			m_uint_range.validate(int_value, range_error);
			validate_uint_limits(int_value, 16, range_error);
			do_pack_uint16(pack_data.get_write_pointer(2), int_value);
			break;

		case ST_uint32:
			m_uint_range.validate(int_value, range_error);
			do_pack_uint32(pack_data.get_write_pointer(4), int_value);
			break;

		case ST_uint64:
			m_uint64_range.validate(int_value, range_error);
			do_pack_uint64(pack_data.get_write_pointer(8), int_value);
			break;

		case ST_float64:
			m_double_range.validate(int_value, range_error);
			do_pack_float64(pack_data.get_write_pointer(8), int_value);
			break;

		default:
			pack_error = true;
	}
}

// pack_int64 packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_int64(PackData &pack_data, int64_t value,
                                 bool &pack_error, bool &range_error) const
{
	int64_t int_value = value * m_divisor;
	if(m_has_modulus && m_uint64_modulus != 0)
	{
		if(int_value < 0)
		{
			int_value = m_uint64_modulus - 1 - (-int_value - 1) % m_uint64_modulus;
		}
		else
		{
			int_value = int_value % m_uint64_modulus;
		}
	}

	switch(m_type)
	{
		case ST_int8:
			m_int_range.validate((int)int_value, range_error);
			validate_int64_limits(int_value, 8, range_error);
			do_pack_int8(pack_data.get_write_pointer(1), (int)int_value);
			break;

		case ST_int16:
			m_int_range.validate((int)int_value, range_error);
			validate_int64_limits(int_value, 16, range_error);
			do_pack_int16(pack_data.get_write_pointer(2), (int)int_value);
			break;

		case ST_int32:
			m_int_range.validate((int)int_value, range_error);
			validate_int64_limits(int_value, 32, range_error);
			do_pack_int32(pack_data.get_write_pointer(4), (int)int_value);
			break;

		case ST_int64:
			m_int64_range.validate(int_value, range_error);
			do_pack_int64(pack_data.get_write_pointer(8), int_value);
			break;

		case ST_char:
		case ST_uint8:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)(uint64_t)int_value, range_error);
			validate_uint64_limits((uint64_t)int_value, 8, range_error);
			do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)(uint64_t)int_value);
			break;

		case ST_uint16:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)(uint64_t)int_value, range_error);
			validate_uint64_limits((uint64_t)int_value, 16, range_error);
			do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)(uint64_t)int_value);
			break;

		case ST_uint32:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint_range.validate((unsigned int)(uint64_t)int_value, range_error);
			validate_uint64_limits((uint64_t)int_value, 32, range_error);
			do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)(uint64_t)int_value);
			break;

		case ST_uint64:
			if(int_value < 0)
			{
				range_error = true;
			}
			m_uint64_range.validate((uint64_t)int_value, range_error);
			do_pack_uint64(pack_data.get_write_pointer(8), (uint64_t)int_value);
			break;

		case ST_float64:
			m_double_range.validate((double)int_value, range_error);
			do_pack_float64(pack_data.get_write_pointer(8), (double)int_value);
			break;

		default:
			pack_error = true;
	}
}

// pack_uint64 packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_uint64(PackData &pack_data, uint64_t value,
                                  bool &pack_error, bool &range_error) const
{
	uint64_t int_value = value * m_divisor;
	if(m_has_modulus && m_uint64_modulus != 0)
	{
		int_value = int_value % m_uint64_modulus;
	}

	switch(m_type)
	{
		case ST_int8:
			if((int64_t)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)(int64_t)int_value, range_error);
			validate_int64_limits((int64_t)int_value, 8, range_error);
			do_pack_int8(pack_data.get_write_pointer(1), (int)(int64_t)int_value);
			break;

		case ST_int16:
			if((int64_t)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)(int64_t)int_value, range_error);
			validate_int64_limits((int64_t)int_value, 16, range_error);
			do_pack_int16(pack_data.get_write_pointer(2), (int)(int64_t)int_value);
			break;

		case ST_int32:
			if((int64_t)int_value < 0)
			{
				range_error = true;
			}
			m_int_range.validate((int)(int64_t)int_value, range_error);
			validate_int64_limits((int64_t)int_value, 32, range_error);
			do_pack_int32(pack_data.get_write_pointer(4), (int)(int64_t)int_value);
			break;

		case ST_int64:
			if((int64_t)int_value < 0)
			{
				range_error = true;
			}
			m_int64_range.validate((int64_t)int_value, range_error);
			do_pack_int64(pack_data.get_write_pointer(8), (int64_t)int_value);
			break;

		case ST_char:
		case ST_uint8:
			m_uint_range.validate((unsigned int)int_value, range_error);
			validate_uint64_limits(int_value, 8, range_error);
			do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)int_value);
			break;

		case ST_uint16:
			m_uint_range.validate((unsigned int)int_value, range_error);
			validate_uint64_limits(int_value, 16, range_error);
			do_pack_uint16(pack_data.get_write_pointer(2), (unsigned int)int_value);
			break;

		case ST_uint32:
			m_uint_range.validate((unsigned int)int_value, range_error);
			validate_uint64_limits(int_value, 32, range_error);
			do_pack_uint32(pack_data.get_write_pointer(4), (unsigned int)int_value);
			break;

		case ST_uint64:
			m_uint64_range.validate(int_value, range_error);
			do_pack_uint64(pack_data.get_write_pointer(8), int_value);
			break;

		case ST_float64:
			m_double_range.validate((double)int_value, range_error);
			do_pack_float64(pack_data.get_write_pointer(8), (double)int_value);
			break;

		default:
			pack_error = true;
	}
}

// pack_string packs the indicated numeric or string value into the stream.
void SimpleParameter::pack_string(PackData &pack_data, const string &value,
                                  bool &pack_error, bool &range_error) const
{
	size_t string_length = value.length();

	switch(m_type)
	{
		case ST_char:
		case ST_uint8:
		case ST_int8:
			if(string_length == 0)
			{
				pack_error = true;
			}
			else
			{
				if(string_length != 1)
				{
					range_error = true;
				}
				m_uint_range.validate((unsigned int)value[0], range_error);
				do_pack_uint8(pack_data.get_write_pointer(1), (unsigned int)value[0]);
			}
			break;

		case ST_string:
		case ST_blob:
			m_uint_range.validate(string_length, range_error);
			validate_uint_limits(string_length, sizeof(length_tag_t) * 8, range_error);
			if(m_num_length_bytes != 0)
			{
				do_pack_length_tag(pack_data.get_write_pointer(sizeof(length_tag_t)), string_length);
			}
			pack_data.append_data(value.data(), string_length);
			break;

		case ST_blob32:
			m_uint_range.validate(string_length, range_error);
			if(m_num_length_bytes != 0)
			{
				do_pack_uint32(pack_data.get_write_pointer(4), string_length);
			}
			pack_data.append_data(value.data(), string_length);
			break;

		default:
			pack_error = true;
	}
}

// pack_default_value packs the simpleParameter's specified default value (or a sensible default
//     if no value is specified) into the stream.  Returns true if the default value is packed,
//     false if the simpleParameter doesn't know how to pack its default value.
bool SimpleParameter::pack_default_value(PackData &pack_data, bool &pack_error) const
{
	if(has_default_value())
	{
		return Field::pack_default_value(pack_data, pack_error);
	}

	if(m_has_nested_fields)
	{
		// If the simple type is an array (or string) type, pack the
		// appropriate length array, with code similar to
		// ArrayParameter::pack_default_value().

		unsigned int minimum_length = 0;
		if(!m_uint_range.is_empty())
		{
			minimum_length = m_uint_range.get_min(0);
		}

		Packer packer;
		packer.begin_pack(this);
		packer.push();
		for(unsigned int i = 0; i < minimum_length; i++)
		{
			packer.pack_default_value();
		}
		packer.pop();
		if(!packer.end_pack())
		{
			pack_error = true;

		}
		else
		{
			pack_data.append_data(packer.get_data(), packer.get_length());
		}

	}
	else
	{
		// Otherwise, if it's just a simple numeric type, pack a zero or
		// the minimum value.
		switch(m_type)
		{
			case ST_int8:
			case ST_int16:
			case ST_int32:
				if(m_int_range.is_in_range(0))
				{
					pack_int(pack_data, 0, pack_error, pack_error);
				}
				else
				{
					pack_int(pack_data, m_int_range.get_min(0), pack_error, pack_error);
				}
				break;

			case ST_int64:
				if(m_int64_range.is_in_range(0))
				{
					pack_int64(pack_data, 0, pack_error, pack_error);
				}
				else
				{
					pack_int64(pack_data, m_int64_range.get_min(0), pack_error, pack_error);
				}
				break;

			case ST_char:
			case ST_uint8:
			case ST_uint16:
			case ST_uint32:
				if(m_uint_range.is_in_range(0))
				{
					pack_uint(pack_data, 0, pack_error, pack_error);
				}
				else
				{
					pack_uint(pack_data, m_uint_range.get_min(0), pack_error, pack_error);
				}
				break;

			case ST_uint64:
				if(m_uint64_range.is_in_range(0))
				{
					pack_uint64(pack_data, 0, pack_error, pack_error);
				}
				else
				{
					pack_uint64(pack_data, m_uint64_range.get_min(0), pack_error, pack_error);
				}
				break;

			case ST_float64:
				if(m_double_range.is_in_range(0.0))
				{
					pack_double(pack_data, 0.0, pack_error, pack_error);
				}
				else
				{
					pack_double(pack_data, m_double_range.get_min(0), pack_error, pack_error);
				}
				break;

			default:
				pack_error = true;
		}
	}
	return true;
}



// unpack_double unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_double(const char *data, size_t length, size_t &p, double &value,
                                    bool &pack_error, bool &range_error) const
{
	switch(m_type)
	{
		case ST_int8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int8(data + p);
			m_int_range.validate(int_value, range_error);
			value = int_value;
			p++;
		}
		break;

		case ST_int16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int16(data + p);
			m_int_range.validate(int_value, range_error);
			value = int_value;
			p += 2;
		}
		break;

		case ST_int32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int32(data + p);
			m_int_range.validate(int_value, range_error);
			value = int_value;
			p += 4;
		}
		break;

		case ST_int64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			int64_t int_value = do_unpack_int64(data + p);
			m_int64_range.validate(int_value, range_error);
			value = (double)int_value;
			p += 8;
		}
		break;

		case ST_char:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint8(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = uint_value;
			p++;
		}
		break;

		case ST_uint16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint16(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = uint_value;
			p += 2;
		}
		break;

		case ST_uint32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint32(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = uint_value;
			p += 4;
		}
		break;

		case ST_uint64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			uint64_t uint_value = do_unpack_uint64(data + p);
			m_uint64_range.validate(uint_value, range_error);
			value = (double)uint_value;
			p += 8;
		}
		break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_float64(data + p);
			m_double_range.validate(value, range_error);
			p += 8;
		}
		break;

		default:
			pack_error = true;
			return;
	}

	if(m_divisor != 1)
	{
		value = value / m_divisor;
	}

	return;
}

// unpack_int unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_int(const char *data, size_t length, size_t &p, int &value,
                                 bool &pack_error, bool &range_error) const
{
	switch(m_type)
	{
		case ST_int8:
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_int8(data + p);
			m_int_range.validate(value, range_error);
			p++;
			break;

		case ST_int16:
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_int16(data + p);
			m_int_range.validate(value, range_error);
			p += 2;
			break;

		case ST_int32:
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_int32(data + p);
			m_int_range.validate(value, range_error);
			p += 4;
			break;

		case ST_int64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			int64_t int_value = do_unpack_uint64(data + p);
			m_int64_range.validate(int_value, range_error);
			value = (int)int_value;
			if(value != int_value)
			{
				// uint exceeded the storage capacity of a signed int.
				pack_error = true;
			}
			p += 8;
		}
		break;

		case ST_char:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint8(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = uint_value;
			p++;
		}
		break;

		case ST_uint16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint16(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (int)uint_value;
			p += 2;
		}
		break;

		case ST_uint32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint32(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (int)uint_value;
			if(value < 0)
			{
				pack_error = true;
			}
			p += 4;
		}
		break;

		case ST_uint64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			uint64_t uint_value = do_unpack_uint64(data + p);
			m_uint64_range.validate(uint_value, range_error);
			value = (int)(unsigned int)uint_value;
			if((unsigned int)value != uint_value || value < 0)
			{
				pack_error = true;
			}
			p += 8;
		}
		break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			double real_value = do_unpack_float64(data + p);
			m_double_range.validate(real_value, range_error);
			value = (int)real_value;
			p += 8;
		}
		break;

		default:
			pack_error = true;
			return;
	}

	if(m_divisor != 1)
	{
		value = value / m_divisor;
	}

	return;
}

// unpack_uint unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_uint(const char *data, size_t length, size_t &p, unsigned int &value,
                                  bool &pack_error, bool &range_error) const
{
	switch(m_type)
	{
		case ST_int8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int8(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (unsigned int)int_value;
			p++;
		}
		break;

		case ST_int16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int16(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (unsigned int)int_value;
			p += 2;
		}
		break;

		case ST_int32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int32(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (unsigned int)int_value;
			p += 4;
		}
		break;

		case ST_int64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			int64_t int_value = do_unpack_int64(data + p);
			m_int64_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (unsigned int)(int)int_value;
			if(value != int_value)
			{
				pack_error = true;
			}
			p += 8;
		}
		break;

		case ST_char:
		case ST_uint8:
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_uint8(data + p);
			m_uint_range.validate(value, range_error);
			p++;
			break;

		case ST_uint16:
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_uint16(data + p);
			m_uint_range.validate(value, range_error);
			p += 2;
			break;

		case ST_uint32:
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_uint32(data + p);
			m_uint_range.validate(value, range_error);
			p += 4;
			break;

		case ST_uint64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			uint64_t uint_value = do_unpack_uint64(data + p);
			m_uint64_range.validate(uint_value, range_error);
			value = (unsigned int)uint_value;
			if(value != uint_value)
			{
				pack_error = true;
			}
			p += 8;
		}
		break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			double real_value = do_unpack_float64(data + p);
			m_double_range.validate(real_value, range_error);
			value = (unsigned int)real_value;
			p += 8;
		}
		break;

		default:
			pack_error = true;
			return;
	}

	if(m_divisor != 1)
	{
		value = value / m_divisor;
	}

	return;
}

// unpack_int64 unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_int64(const char *data, size_t length, size_t &p, int64_t &value,
                                   bool &pack_error, bool &range_error) const
{
	switch(m_type)
	{
		case ST_int8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int8(data + p);
			m_int_range.validate(int_value, range_error);
			value = (int64_t)int_value;
			p++;
		}
		break;

		case ST_int16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int16(data + p);
			m_int_range.validate(int_value, range_error);
			value = (int64_t)int_value;
			p += 2;
		}
		break;

		case ST_int32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int32(data + p);
			m_int_range.validate(int_value, range_error);
			value = (int64_t)int_value;
			p += 4;
		}
		break;

		case ST_int64:
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_int64(data + p);
			m_int64_range.validate(value, range_error);
			p += 8;
			break;

		case ST_char:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint8(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (int64_t)(int)uint_value;
			p++;
		}
		break;

		case ST_uint16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint16(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (int64_t)(int)uint_value;
			p += 2;
		}
		break;

		case ST_uint32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint32(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (int64_t)(int)uint_value;
			p += 4;
		}
		break;

		case ST_uint64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			uint64_t uint_value = do_unpack_uint64(data + p);
			m_uint64_range.validate(uint_value, range_error);
			value = (int64_t)uint_value;
			if(value < 0)
			{
				pack_error = true;
			}
			p += 8;
		}
		break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			double real_value = do_unpack_float64(data + p);
			m_double_range.validate(real_value, range_error);
			value = (int64_t)real_value;
			p += 8;
		}
		break;

		default:
			pack_error = true;
			return;
	}

	if(m_divisor != 1)
	{
		value = value / m_divisor;
	}

	return;
}

// unpack_uint64 unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_uint64(const char *data, size_t length, size_t &p, uint64_t &value,
                                    bool &pack_error, bool &range_error) const
{
	switch(m_type)
	{
		case ST_int8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int8(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (uint64_t)(unsigned int)int_value;
			p++;
		}
		break;

		case ST_int16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int16(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (uint64_t)(unsigned int)int_value;
			p += 2;
		}
		break;

		case ST_int32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			int int_value = do_unpack_int32(data + p);
			m_int_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (uint64_t)(unsigned int)int_value;
			p += 4;
		}
		break;

		case ST_int64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			int64_t int_value = do_unpack_int64(data + p);
			m_int64_range.validate(int_value, range_error);
			if(int_value < 0)
			{
				pack_error = true;
			}
			value = (uint64_t)int_value;
			p += 8;
		}
		break;

		case ST_char:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint8(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (uint64_t)uint_value;
			p++;
		}
		break;

		case ST_uint16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint16(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (uint64_t)uint_value;
			p += 2;
		}
		break;

		case ST_uint32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int uint_value = do_unpack_uint32(data + p);
			m_uint_range.validate(uint_value, range_error);
			value = (uint64_t)uint_value;
			p += 4;
		}
		break;

		case ST_uint64:
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			value = do_unpack_uint64(data + p);
			m_uint64_range.validate(value, range_error);
			p += 8;
			break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return;
			}
			double real_value = do_unpack_float64(data + p);
			m_double_range.validate(real_value, range_error);
			value = (uint64_t)real_value;
			p += 8;
		}
		break;

		default:
			pack_error = true;
			return;
	}

	if(m_divisor != 1)
	{
		value = value / m_divisor;
	}

	return;
}

// unpack_string unpacks the current numeric or string value from the stream.
void SimpleParameter::unpack_string(const char *data, size_t length, size_t &p, string &value,
                                    bool &pack_error, bool &range_error) const
{
	// If the type is a single byte, unpack it into a string of length 1.
	switch(m_type)
	{
		case ST_char:
		case ST_int8:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return;
			}
			unsigned int int_value = do_unpack_uint8(data + p);
			m_uint_range.validate(int_value, range_error);
			value.assign(1, int_value);
			p++;
		}
		return;

		default:
			break;
	}

	size_t string_length;

	if(m_num_length_bytes == 0)
	{
		string_length = m_fixed_byte_size;

	}
	else
	{
		switch(m_type)
		{
			case ST_string:
			case ST_blob:
				if(p + sizeof(length_tag_t) > length)
				{
					pack_error = true;
					return;
				}
				string_length = do_unpack_length_tag(data + p);
				p += sizeof(length_tag_t);
				break;

			case ST_blob32:
				if(p + 4 > length)
				{
					pack_error = true;
					return;
				}
				string_length = do_unpack_uint32(data + p);
				p += 4;
				break;

			default:
				pack_error = true;
				return;
		}
	}

	m_uint_range.validate(string_length, range_error);

	if(p + string_length > length)
	{
		pack_error = true;
		return;
	}
	value.assign(data + p, string_length);
	p += string_length;

	return;
}

// unpack_validate internally unpacks the current numeric or string value and validates it against
//     the type range limits, ut does not return the value.  Returns true on success,
//     false on failure (e.g. we don't know how to validate this field).
bool SimpleParameter::unpack_validate(const char *data, size_t length, size_t &p,
                                      bool &pack_error, bool &range_error) const
{
	if(!m_has_range_limits)
	{
		return unpack_skip(data, length, p, pack_error);
	}
	switch(m_type)
	{
		case ST_int8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return true;
			}
			int int_value = do_unpack_int8(data + p);
			m_int_range.validate(int_value, range_error);
			p++;
		}
		break;

		case ST_int16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return true;
			}
			int int_value = do_unpack_int16(data + p);
			m_int_range.validate(int_value, range_error);
			p += 2;
		}
		break;

		case ST_int32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return true;
			}
			int int_value = do_unpack_int32(data + p);
			m_int_range.validate(int_value, range_error);
			p += 4;
		}
		break;

		case ST_int64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return true;
			}
			int64_t int_value = do_unpack_int64(data + p);
			m_int64_range.validate(int_value, range_error);
			p += 8;
		}
		break;

		case ST_char:
		case ST_uint8:
		{
			if(p + 1 > length)
			{
				pack_error = true;
				return true;
			}
			unsigned int uint_value = do_unpack_uint8(data + p);
			m_uint_range.validate(uint_value, range_error);
			p++;
		}
		break;

		case ST_uint16:
		{
			if(p + 2 > length)
			{
				pack_error = true;
				return true;
			}
			unsigned int uint_value = do_unpack_uint16(data + p);
			m_uint_range.validate(uint_value, range_error);
			p += 2;
		}
		break;

		case ST_uint32:
		{
			if(p + 4 > length)
			{
				pack_error = true;
				return true;
			}
			unsigned int uint_value = do_unpack_uint32(data + p);
			m_uint_range.validate(uint_value, range_error);
			p += 4;
		}
		break;

		case ST_uint64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return true;
			}
			uint64_t uint_value = do_unpack_uint64(data + p);
			m_uint64_range.validate(uint_value, range_error);
			p += 8;
		}
		break;

		case ST_float64:
		{
			if(p + 8 > length)
			{
				pack_error = true;
				return true;
			}
			double real_value = do_unpack_float64(data + p);
			m_double_range.validate(real_value, range_error);
			p += 8;
		}
		break;

		case ST_string:
		case ST_blob:
			if(m_num_length_bytes == 0)
			{
				p += m_fixed_byte_size;

			}
			else
			{
				if(p + sizeof(length_tag_t) > length)
				{
					pack_error = true;
					return true;
				}
				size_t string_length = do_unpack_length_tag(data + p);
				m_uint_range.validate(string_length, range_error);
				p += sizeof(length_tag_t) + string_length;
			}
			break;

		case ST_blob32:
			if(m_num_length_bytes == 0)
			{
				p += m_fixed_byte_size;

			}
			else
			{
				if(p + 4 > length)
				{
					pack_error = true;
					return true;
				}
				size_t string_length = do_unpack_uint32(data + p);
				m_uint_range.validate(string_length, range_error);
				p += 4 + string_length;
			}
			break;

		default:
			return false;
	}

	return true;
}

// unpack_skip increments p to the end of the current field without actually unpacking any data or
//     performing any range validation.  Returns true on success, false on failure.
bool SimpleParameter::unpack_skip(const char *data, size_t length, size_t &p, bool &pack_error) const
{
	size_t string_length;

	switch(m_type)
	{
		case ST_char:
		case ST_int8:
		case ST_uint8:
			p++;
			break;

		case ST_int16:
		case ST_uint16:
			p += 2;
			break;

		case ST_int32:
		case ST_uint32:
			p += 4;
			break;

		case ST_int64:
		case ST_uint64:
		case ST_float64:
			p += 8;
			break;

		case ST_string:
		case ST_blob:
			if(m_num_length_bytes == 0)
			{
				p += m_fixed_byte_size;

			}
			else
			{
				if(p + sizeof(length_tag_t) > length)
				{
					return false;
				}
				string_length = do_unpack_length_tag(data + p);
				p += sizeof(length_tag_t) + string_length;
			}
			break;

		case ST_blob32:
			if(m_num_length_bytes == 0)
			{
				p += m_fixed_byte_size;

			}
			else
			{
				if(p + 4 > length)
				{
					return false;
				}
				string_length = do_unpack_uint32(data + p);
				p += 4 + string_length;
			}
			break;

		default:
			return false;
	}

	if(p > length)
	{
		pack_error = true;
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
		out << m_type;
		if(m_has_modulus)
		{
			out << "%" << m_orig_modulus;
		}
		if(m_divisor != 1)
		{
			out << "/" << m_divisor;
		}

		switch(m_type)
		{
			case ST_int8:
			case ST_int16:
			case ST_int32:
				if(!m_int_range.is_empty())
				{
					out << "(";
					m_int_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case ST_int64:
				if(!m_int64_range.is_empty())
				{
					out << "(";
					m_int64_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case ST_uint8:
			case ST_uint16:
			case ST_uint32:
				if(!m_uint_range.is_empty())
				{
					out << "(";
					m_uint_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case ST_char:
				if(!m_uint_range.is_empty())
				{
					out << "(";
					m_uint_range.output_char(out, m_divisor);
					out << ")";
				}
				break;

			case ST_uint64:
				if(!m_uint64_range.is_empty())
				{
					out << "(";
					m_uint64_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case ST_float64:
				if(!m_double_range.is_empty())
				{
					out << "(";
					m_double_range.output(out, m_divisor);
					out << ")";
				}
				break;

			case ST_string:
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

	hashgen.add_int(m_type);
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

// do_check_match returns true if the other interface is bitwise the same as this one;
//     that is, a uint32 only matches a uint32, etc. Names of components, and range limits, are not compared.
bool SimpleParameter::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_simple_parameter(this);
}

// do_check_match_simple_parameter returns true if this field
//     matches the indicated simple parameter, false otherwise.
bool SimpleParameter::do_check_match_simple_parameter(const SimpleParameter *other) const
{
	if(m_divisor != other->m_divisor)
	{
		return false;
	}

	if(m_type == other->m_type)
	{
		return true;
	}

	// Check for certain types that are considered equivalent to each
	// other.
	switch(m_type)
	{
		case ST_uint8:
		case ST_char:
			switch(other->m_type)
			{
				case ST_uint8:
				case ST_char:
					return true;

				default:
					return false;
			}

		case ST_string:
		case ST_blob:
		case ST_uint8array:
			switch(other->m_type)
			{
				case ST_string:
				case ST_blob:
				case ST_uint8array:
					return true;

				default:
					return false;
			}

		default:
			return false;
	}
}

// do_check_match_array_parameter returns true if this field matches
//     the indicated array parameter, false otherwise.
bool SimpleParameter::do_check_match_array_parameter(const ArrayParameter *other) const
{
	if(other->get_array_size() != -1)
	{
		// We cannot match a fixed-size array.
		return false;
	}
	if(m_nested_field == NULL)
	{
		// Only an array-style simple parameter can match a ArrayParameter.
		return false;
	}

	return m_nested_field->check_match(other->get_element_type());
}

// create_nested_field creates the one instance of the SimpleParameter corresponding to this
//     combination of type and divisor if it is not already created.
SimpleParameter *SimpleParameter::create_nested_field(SubatomicType type, unsigned int divisor)
{
	std::map<unsigned int, SimpleParameter*> &divisor_map = cls_nested_field_map[type];
	auto div_it = divisor_map.find(divisor);
	if(div_it != divisor_map.end())
	{
		return div_it->second;
	}

	SimpleParameter *nested_field = new SimpleParameter(type, divisor);
	divisor_map[divisor] = nested_field;
	return nested_field;
}

// create_uint32uint8_type creates the one instance of the Uint32Uint8Type object if it is not already created.
PackerInterface *SimpleParameter::create_uint32uint8_type()
{
	if(cls_uint32uint8_type == NULL)
	{
		Class *dclass = new Class(NULL, "", true, false);
		dclass->add_field(new SimpleParameter(ST_uint32));
		dclass->add_field(new SimpleParameter(ST_uint8));
		cls_uint32uint8_type = new ClassParameter(dclass);
	}
	return cls_uint32uint8_type;
}


} // close namespace dclass
