// Filename: SwitchParameter.cpp
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "SwitchParameter.h"
#include "Switch.h"
#include "HashGenerator.h"
namespace dclass   // open namespace dclass
{


// switch constructor
SwitchParameter::SwitchParameter(const Switch *dswitch) : m_switch(dswitch)
{
	set_name(dswitch->get_name());

	m_has_fixed_byte_size = true;
	m_fixed_byte_size = 0;
	m_has_fixed_structure = false;

	// The Switch presents just one nested field initially, which is
	// the key parameter.  When we pack or unpack that, the Packer
	// calls apply_switch(), which returns a new record that presents
	// the remaining nested fields.
	m_has_nested_fields = true;
	m_num_nested_fields = 1;

	m_pack_type = PT_switch;

	Field *key_parameter = dswitch->get_key_parameter();
	m_has_fixed_byte_size = m_has_fixed_byte_size && key_parameter->has_fixed_byte_size();
	m_has_range_limits = m_has_range_limits || key_parameter->has_range_limits();
	m_has_default_value = m_has_default_value || key_parameter->has_default_value();

	int num_cases = m_switch->get_num_cases();
	if(num_cases > 0)
	{
		m_fixed_byte_size = m_switch->get_case(0)->get_fixed_byte_size();

		// Consider each case for fixed size, etc.
		for(int i = 0; i < num_cases; i++)
		{
			const Switch::SwitchFields *fields =
			    (const Switch::SwitchFields *)m_switch->get_case(i);

			if(!fields->has_fixed_byte_size() ||
			        fields->get_fixed_byte_size() != m_fixed_byte_size)
			{

				// Nope, we have a variable byte size.
				m_has_fixed_byte_size = false;
			}

			m_has_range_limits = m_has_range_limits || fields->has_range_limits();
			m_has_default_value = m_has_default_value || fields->m_has_default_value;
		}
	}

	// Also consider the default case, if there is one.
	const Switch::SwitchFields *fields =
	    (Switch::SwitchFields *)m_switch->get_default_case();
	if(fields != (Switch::SwitchFields *)NULL)
	{
		if(!fields->has_fixed_byte_size() ||
		        fields->get_fixed_byte_size() != m_fixed_byte_size)
		{
			m_has_fixed_byte_size = false;
		}

		m_has_range_limits = m_has_range_limits || fields->has_range_limits();
		m_has_default_value = m_has_default_value || fields->m_has_default_value;
	}
}

// copy constructor
SwitchParameter::SwitchParameter(const SwitchParameter &copy) :
	Parameter(copy), m_switch(copy.m_switch)
{
}

// as_switch_parameter returns the same parameter pointer converted to a switch parameter,
//     if this is in fact a switch parameter; otherwise, returns NULL.
SwitchParameter *SwitchParameter::as_switch_parameter()
{
	return this;
}

// as_switch_parameter returns the same parameter pointer converted to a switch parameter,
//     if this is in fact a switch parameter; otherwise, returns NULL.
const SwitchParameter *SwitchParameter::as_switch_parameter() const
{
	return this;
}

// make_copy returns a parameter copy of this SwitchParameter
Parameter *SwitchParameter::make_copy() const
{
	return new SwitchParameter(*this);
}

// is_valid returns false if the type is an invalid type.
bool SwitchParameter::is_valid() const
{
	return true; //_dswitch->is_valid();
}

// get_switch returns the switch object this parameter represents.
const Switch *SwitchParameter::get_switch() const
{
	return m_switch;
}

// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     This returns NULL if n is not within the range 0 <= n < get_num_nested_fields()).
PackerInterface *SwitchParameter::get_nested_field(int) const
{
	return m_switch->get_key_parameter();
}

// apply_switch returns the PackerInterface that presents the alternative fields for the
//     case indicated by the given packed value string, or NULL if the value string does not match
//     one of the expected cases.
const PackerInterface *SwitchParameter::apply_switch(const char *value_data, size_t length) const
{
	return m_switch->apply_switch(value_data, length);
}

// output_instance formats the parameter in the C++-like dc syntax as a typename and identifier.
void SwitchParameter::output_instance(std::ostream &out, bool brief, const std::string &prename,
                                      const std::string &name, const std::string &postname) const
{
	if(get_typedef() != (Typedef *)NULL)
	{
		output_typedef_name(out, brief, prename, name, postname);

	}
	else
	{
		m_switch->output_instance(out, brief, prename, name, postname);
	}
}

// write_instance formats the parameter in the C++-like dc syntax as a typename and identifier.
void SwitchParameter::write_instance(std::ostream &out, bool brief, int indent_level,
                                     const std::string &prename, const std::string &name,
                                     const std::string &postname) const
{
	if(get_typedef() != (Typedef *)NULL)
	{
		write_typedef_name(out, brief, indent_level, prename, name, postname);

	}
	else
	{
		m_switch->write_instance(out, brief, indent_level, prename, name, postname);
	}
}

// generate_hash accumulates the properties of this type into the hash.
void SwitchParameter::generate_hash(HashGenerator &hashgen) const
{
	Parameter::generate_hash(hashgen);
	m_switch->generate_hash(hashgen);
}

// pack_default_value packs the switchParameter's specified default value (or a sensible default
//     if no value is specified) into the stream.  Returns true if the default value is packed,
//     false if the switchParameter doesn't know how to pack its default value.
bool SwitchParameter::pack_default_value(PackData &pack_data, bool &pack_error) const
{
	if(has_default_value())
	{
		return Field::pack_default_value(pack_data, pack_error);
	}

	return m_switch->pack_default_value(pack_data, pack_error);
}

// do_check_match returns true if the other interface is bitwise the same as this one--that is,
//     a uint32 only matches a uint32, etc. Names of components, and range limits, are not compared.
bool SwitchParameter::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_switch_parameter(this);
}

// do_check_match_switch_parameter returns true if this field matches the indicated switch parameter, false otherwise.
bool SwitchParameter::do_check_match_switch_parameter(const SwitchParameter *other) const
{
	return m_switch->do_check_match_switch(other->m_switch);
}


} // close namespace dclass
