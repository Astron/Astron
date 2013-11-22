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
ArrayParameter::ArrayParameter(DCParameter *element_type, const UintRange &size) :
	m_element_type(element_type), m_array_size_range(size)
{
	set_name(m_element_type->get_name());
	m_element_type->set_name(string());

	m_array_size = -1;
	if(m_array_size_range.has_one_value())
	{
		m_array_size = m_array_size_range.get_one_value();
	}
	else
	{
		_has_range_limits = true;
	}

	if(m_array_size >= 0 && m_element_type->has_fixed_byte_size())
	{
		_has_fixed_byte_size = true;
		_fixed_byte_size = m_array_size * m_element_type->get_fixed_byte_size();
		_has_fixed_structure = true;

	}
	else
	{
		// We only need to store the length bytes if the array has a
		// variable size.
		_num_length_bytes = sizeof(length_tag_t);
	}

	if(m_element_type->has_range_limits())
	{
		_has_range_limits = true;
	}

	if(m_element_type->has_default_value())
	{
		_has_default_value = true;
	}

	_has_nested_fields = true;
	_num_nested_fields = m_array_size;
	_pack_type = PT_array;

	DCSimpleParameter *simple_type = m_element_type->as_simple_parameter();
	if(simple_type != (SimpleParameter *)NULL)
	{
		if(simple_type->get_type() == ST_char)
		{
			// We make a special case for char[] arrays: these we format as
			// a string.  (It will still accept an array of ints packed into
			// it.)  We don't make this special case for uint8[] or int8[]
			// arrays, although we will accept a string packed in for them.
			_pack_type = PT_string;
		}
	}
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
Parameter* ArrayParameter::append_array_specification(const UintRange &size)
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

// calc_num_nested_fields returns the number of nested fields to expect,
//     given a certain length in bytes (as read from a get_num_length_bytes()).
//     This should only be called if get_num_length_bytes() returns non-zero.
//     Note: This is primarily used in unpacking to determine the number of elements in an array.
int ArrayParameter::calc_num_nested_fields(size_t length_bytes) const
{
	if(m_element_type->has_fixed_byte_size())
	{
		return length_bytes / m_element_type->get_fixed_byte_size();
	}
	return -1;
}


// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     The return is NULL if 'n' is out-of-bounds of 0 <= n < get_num_nested_fields().
PackerInterface* ArrayParameter::get_nested_field(int) const
{
	return m_element_type;
}

// validate_num_nested_fields determines whether the number of nested fields added while packing
//     an array-type parameter is valid for this type.  This is called by the packer after a
//     number of fields have been packed via push() .. pack_*() .. pop().
//     Note: This is primarily useful for array types with dynamic ranges that
//           can't validate the number of fields any other way.
bool ArrayParameter::validate_num_nested_fields(int num_nested_fields) const
{
	bool range_error = false;
	m_array_size_range.validate(num_nested_fields, range_error);

	return !range_error;
}

// output_instance formats the parameter to the syntax of an array parameter in a .dc file
//     as TYPE IDENTIFIER[RANGE] with optional IDENTIFIER and RANGE,
//     and outputs the formatted string to the stream.
void ArrayParameter::output_instance(ostream &out, bool brief, const string &prename,
                                     const string &name, const string &postname) const
{
	if(get_typedef() != (DCTypedef *)NULL)
	{
		output_typedef_name(out, brief, prename, name, postname);

	}
	else
	{
		ostringstream strm;

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

// pack_string packs the indicated numeric or string value into the stream.
void ArrayParameter::pack_string(DCPackData &pack_data, const string &value,
                                 bool &pack_error, bool &range_error) const
{
	// We can only pack a string if the array element type is char or int8_t.
	SimpleParameter *simple_type = m_element_type->as_simple_parameter();
	if(simple_type == (SimpleParameter*)NULL)
	{
		pack_error = true;
		return;
	}

	size_t string_length = value.length();

	switch(simple_type->get_type())
	{
		case ST_char:
		case ST_uint8:
		case ST_int8:
			m_array_size_range.validate(string_length, range_error);
			if(_num_length_bytes != 0)
			{
				nassertv(_num_length_bytes == sizeof(length_tag_t));
				do_pack_uint16(pack_data.get_write_pointer(sizeof(length_tag_t)), string_length);
			}
			pack_data.append_data(value.data(), string_length);
			break;

		default:
			pack_error = true;
	}
}

// pack_default_value packs the ArrayParameter's specified default value
//     (or a sensible default if no value is specified) into the stream.
//     Returns true if the default value is packed, or false if the
//     ArrayParameter doesn't know how to pack its default value.
bool ArrayParameter::pack_default_value(PackData &pack_data, bool &pack_error) const
{
	// We only want to call up if the Field can pack the value immediately --
	// we don't trust the Field to generate the default value (since it doesn't
	// know how large the minimum length array is).
	if(_has_default_value && !_default_value_stale)
	{
		return Field::pack_default_value(pack_data, pack_error);
	}

	// If a default value is not specified for a variable-length array,
	// the default is the minimum array.
	unsigned int minimum_length = 0;
	if(!m_array_size_range.is_empty())
	{
		minimum_length = m_array_size_range.get_min(0);
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

	return true;
}

// unpack_string unpacks the current numeric or string value from the stream.
void ArrayParameter::unpack_string(const char *data, size_t length, size_t &p, string &value,
                                   bool &pack_error, bool &range_error) const
{
	// We can only unpack a string if the array element type is char or int8_t.
	SimpleParameter *simple_type = m_element_type->as_simple_parameter();
	if(simple_type == (SimpleParameter*)NULL)
	{
		pack_error = true;
		return;
	}

	size_t string_length;

	switch(simple_type->get_type())
	{
		case ST_char:
		case ST_uint8:
		case ST_int8:
			if(_num_length_bytes != 0)
			{
				string_length = do_unpack_length_tag(data + p);
				p += sizeof(length_tag_t);
			}
			else
			{
				nassertv(m_array_size >= 0);
				string_length = m_array_size;
			}
			if(p + string_length > length)
			{
				pack_error = true;
				return;
			}
			value.assign(data + p, string_length);
			p += string_length;
			break;

		default:
			pack_error = true;
	}
}

// do_check_match returns true if the other interface is bitwise the same as
//     this one--that is, a uint32 only matches a uint32, etc.
//     Names of components, and range limits, are not compared.
bool ArrayParameter::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_array_parameter(this);
}

// do_check_match_simple_parameter returns true if this field matches the
//     indicated simple parameter, or false otherwise.
bool ArrayParameter::do_check_match_simple_parameter(const SimpleParameter *other) const
{
	return ((const PackerInterface*)other)->do_check_match_array_parameter(this);
}

// do_check_match_class_parameter returns true if this field matches the
//     indicated class parameter, or false otherwise.
bool ArrayParameter::do_check_match_class_parameter(const ClassParameter *other) const
{
	return ((const PackerInterface*)other)->do_check_match_array_parameter(this);
}

// do_check_match_array_parameter returns true if this field matches the
//    indicated array parameter, or false otherwise.
bool ArrayParameter::do_check_match_array_parameter(const ArrayParameter *other) const
{
	if(m_array_size != other->m_array_size)
	{
		return false;
	}
	return m_element_type->check_match(other->m_element_type);
}


} // close namespace dclass
