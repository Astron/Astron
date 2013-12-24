// Filename: Field.cpp
// Created by: drose (11 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Field.h"
#include "File.h"
#include "Packer.h"
#include "Class.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// nameless constructor (for structs)
Field::Field() : m_class(NULL), m_number(-1), m_default_value_stale(true),
	m_has_default_value(false), m_bogus_field(false)
{
	m_has_nested_fields = true;
	m_num_nested_fields = 0;
	m_pack_type = PT_field;
	m_has_fixed_byte_size = true;
	m_fixed_byte_size = 0;
}

// named constructor (for classes)
Field::Field(const std::string &name, Class *dclass) : PackerInterface(name),
	m_class(dclass), m_number(-1), m_default_value_stale(true),
	m_has_default_value(false), m_bogus_field(false)
{
	m_has_nested_fields = true;
	m_num_nested_fields = 0;
	m_pack_type = PT_field;
	m_has_fixed_byte_size = true;
	m_fixed_byte_size = 0;
}

// destructor
Field::~Field()
{
}

// as_field returns the same pointer converted to a field pointer,
//     if this is in fact a field; otherwise, returns NULL.
Field* Field::as_field()
{
	return this;
}

// as_field returns the same pointer converted to a field pointer,
//     if this is in fact a field; otherwise, returns NULL.
const Field* Field::as_field() const
{
	return this;
}

// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
AtomicField* Field::as_atomic_field()
{
	return (AtomicField*)NULL;
}

// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
const AtomicField* Field::as_atomic_field() const
{
	return (AtomicField*)NULL;
}

// as_molecular_field returns the same field pointer converted to a molecular field
//     pointer, if this is in fact a molecular field; otherwise, returns NULL.
MolecularField* Field::as_molecular_field()
{
	return (MolecularField*)NULL;
}

// as_molecular_field returns the same field pointer converted to a molecular field
//     pointer, if this is in fact a molecular field; otherwise, returns NULL.
const MolecularField* Field::as_molecular_field() const
{
	return (MolecularField*)NULL;
}

// as_parameter returns the same field pointer converted to a parameter
//     pointer, if this is in fact a parameter; otherwise, returns NULL.
Parameter* Field::as_parameter()
{
	return (Parameter*)NULL;
}

// as_parameter returns the same field pointer converted to a parameter
//     pointer, if this is in fact a parameter; otherwise, returns NULL.
const Parameter* Field::as_parameter() const
{
	return (Parameter*)NULL;
}

// format_data accepts a blob that represents the packed data for this field,
//     returns a std::string formatting it for human consumption.
//     Returns empty std::string if there is an error.
std::string Field::format_data(const std::string &packed_data, bool show_field_names)
{
	Packer packer;
	packer.set_unpack_data(packed_data);
	packer.begin_unpack(this);
	std::string result = packer.unpack_and_format(show_field_names);
	if(!packer.end_unpack())
	{
		return std::string();
	}
	return result;
}

// parse_string given a human-formatted std::string (for instance, as
//     returned by format_data(), above) that represents the value of this field,
//     parse the std::string and return the corresponding packed data.
//     Returns empty std::string if there is an error.
std::string Field::parse_string(const std::string &formatted_string)
{
	Packer packer;
	packer.begin_pack(this);
	if(!packer.parse_and_pack(formatted_string))
	{
		// Parse error.
		return std::string();
	}
	if(!packer.end_pack())
	{
		// Data type mismatch.
		return std::string();
	}

	return packer.get_string();
}

// validate_ranges verifies that all of the packed values in the field data are
//     within the specified ranges and that there are no extra bytes on the end
//     of the record.  Returns true if all fields are valid, false otherwise.
bool Field::validate_ranges(const std::string &packed_data) const
{
	Packer packer;
	packer.set_unpack_data(packed_data);
	packer.begin_unpack(this);
	packer.unpack_validate();
	if(!packer.end_unpack())
	{
		return false;
	}

	return (packer.get_num_unpacked_bytes() == packed_data.length());
}

// generate_hash accumulates the properties of this field into the hash.
void Field::generate_hash(HashGenerator &hashgen) const
{
	// It shouldn't be necessary to explicitly add _number to the
	// hash--this is computed based on the relative position of this
	// field with the other fields, so adding it explicitly will be
	// redundant.  However, the field name is significant.
	hashgen.add_string(m_name);

	// Actually, we add _number anyway, since we need to ensure the hash
	// code comes out different in the dc_multiple_inheritance case.
	hashgen.add_int(m_number);
}

// pack_default_value packs the field's specified default value (or a sensible default
//     if no value is specified) into the stream. Returns true if the default
//     value is packed, false if the field doesn't know how to pack its default value.
bool Field::pack_default_value(PackData &pack_data, bool &) const
{
	// The default behavior is to pack the default value if we got it;
	// otherwise, to return false and let the packer visit our nested elements.
	if(!m_default_value_stale)
	{
		pack_data.append_data(m_default_value.data(), m_default_value.length());
		return true;
	}

	return false;
}

// set_name sets the name of this field.
void Field::set_name(const std::string &name)
{
	PackerInterface::set_name(name);
	if(m_class != (Class *)NULL)
	{
		m_class->m_file->mark_inherited_fields_stale();
	}
}

// refresh_default_value recomputes the default value of the field by repacking it.
void Field::refresh_default_value()
{
	Packer packer;
	packer.begin_pack(this);
	packer.pack_default_value();
	if(!packer.end_pack())
	{
		std::cerr << "Error while packing default value for " << get_name() << "\n";
	}
	else
	{
		m_default_value.assign(packer.get_data(), packer.get_length());
	}
	m_default_value_stale = false;
}


} // close namespace dclass
