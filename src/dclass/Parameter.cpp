// Filename: Parameter.cpp
// Created by: drose (15 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Parameter.h"
#include "ArrayParameter.h"
#include "HashGenerator.h"
#include "indent.h"
#include "Typedef.h"
namespace dclass   // open namespace dclass
{


// null constructor
Parameter::Parameter()
{
	m_typedef = NULL;
	m_has_fixed_byte_size = false;
	m_has_fixed_structure = false;
	m_num_nested_fields = -1;
}

// copy constructor
Parameter::Parameter(const Parameter &copy) : Field(copy), m_typedef(copy.m_typedef)
{
}

// destructor
Parameter::~Parameter()
{
}

// as_parameter returns the same field pointer converted to a parameter,
//     if this is in fact a parameter; otherwise, returns NULL.
Parameter *Parameter::as_parameter()
{
	return this;
}

// as_parameter returns the same field pointer converted to a parameter,
//     if this is in fact a parameter; otherwise, returns NULL.
const Parameter *Parameter::as_parameter() const
{
	return this;
}

// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
//     if this is in fact a simple parameter; otherwise, returns NULL.
SimpleParameter *Parameter::as_simple_parameter()
{
	return NULL;
}

// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
//     if this is in fact a simple parameter; otherwise, returns NULL.
const SimpleParameter *Parameter::as_simple_parameter() const
{
	return NULL;
}

// as_class_parameter returns the same parameter pointer converted to a class parameter,
//     if this is in fact a class parameter; otherwise, returns NULL.
ClassParameter *Parameter::as_class_parameter()
{
	return NULL;
}

// as_class_parameter returns the same parameter pointer converted to a class parameter,
//     if this is in fact a class parameter; otherwise, returns NULL.
const ClassParameter *Parameter::as_class_parameter() const
{
	return NULL;
}

// as_switch_parameter returns the same parameter pointer converted to a switch parameter,
//     if this is in fact a switch parameter; otherwise, returns NULL.
SwitchParameter *Parameter::as_switch_parameter()
{
	return NULL;
}

// as_switch_parameter returns the same parameter pointer converted to a switch parameter,
//     if this is in fact a switch parameter; otherwise, returns NULL.
const SwitchParameter *Parameter::as_switch_parameter() const
{
	return NULL;
}

// as_array_parameter returns the same parameter pointer converted to an array parameter,
//     if this is in fact an array parameter; otherwise, returns NULL.
ArrayParameter *Parameter::as_array_parameter()
{
	return NULL;
}

// as_array_parameter returns the same parameter pointer converted to an array parameter,
//     if this is in fact an array parameter; otherwise, returns NULL.
const ArrayParameter *Parameter::as_array_parameter() const
{
	return NULL;
}

// get_typedef returns the Typedef instance if this type has been referenced from a typedef, or NULL.
const Typedef *Parameter::get_typedef() const
{
	return m_typedef;
}

// set_typedef records the Typedef object that generated this parameter.
//     This is normally called only from Typedef::make_new_parameter().
void Parameter::set_typedef(const Typedef *dtypedef)
{
	m_typedef = dtypedef;
}

// append_array_specification returns the type represented by this_type[size].
//     In the case of a generic Parameter, it returns an ArrayParameter wrapped around this type.
Parameter *Parameter::append_array_specification(const UnsignedIntRange &size)
{
	return new ArrayParameter(this, size);
}

// output writes a representation of the parameter to an output stream
void Parameter::output(std::ostream &out, bool brief) const
{
	std::string name;
	if(!brief)
	{
		name = get_name();
	}
	output_instance(out, brief, "", name, "");
}

// write writes a representation of the parameter to an output stream
void Parameter::write(std::ostream &out, bool brief, int indent_level) const
{
	// we must always output the name when the parameter occurs by
	// itself within a class, so we pass get_name() even if brief is
	// true.
	write_instance(out, brief, indent_level, "", get_name(), "");
}

// write_instance formats the parameter in the C++-like dc syntax as a typename and identifier.
void Parameter::write_instance(std::ostream &out, bool brief, int indent_level,
                               const std::string &prename, const std::string &name,
                               const std::string &postname) const
{
	indent(out, indent_level);
	output_instance(out, brief, prename, name, postname);
	output_keywords(out);
	out << ";";
	if(!brief && m_number >= 0)
	{
		out << "  // field " << m_number;
	}
	out << "\n";
}

// output_typedef_name formats the instance like output_instance, but uses the typedef name instead.
void Parameter::output_typedef_name(std::ostream &out, bool, const std::string &prename,
                                    const std::string &name, const std::string &postname) const
{
	out << get_typedef()->get_name();
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
}

// write_typedef_name formats the instance like write_instance, but uses the typedef name instead.
void Parameter::write_typedef_name(std::ostream &out, bool brief, int indent_level,
                                   const std::string &prename, const std::string &name,
                                   const std::string &postname) const
{
	indent(out, indent_level) << get_typedef()->get_name();
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
	output_keywords(out);
	out << ";";
	if(!brief && m_number >= 0)
	{
		out << "  // field " << m_number;
	}
	out << "\n";
}

// generate_hash accumulates the properties of this type into the hash.
void Parameter::generate_hash(HashGenerator &hashgen) const
{
	// We specifically don't call up to Field::generate_hash(), since
	// the parameter name is not actually significant to the hash.

	if(get_num_keywords() != 0)
	{
		KeywordList::generate_hash(hashgen);
	}
}


} // close namespace dclass
