// Filename: AtomicField.cpp
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "AtomicField.h"
#include "SimpleParameter.h"
#include "HashGenerator.h"
#include "Packer.h"
#include "indent.h"
#include <math.h>
namespace dclass   // open namespace
{


// constructor
AtomicField::AtomicField(const string &name, Class *dcc, bool bogus_field) : Field(name, dcc)
{
	m_bogus_field = bogus_field;
}

// destructor
AtomicField::~AtomicField()
{
	for(auto it = m_elements.begin(); it != m_elements.end(); ++it)
	{
		delete(*it);
	}
	m_elements.clear();
}

// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
AtomicField* AtomicField::as_atomic_field()
{
	return this;
}
// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
const AtomicField* AtomicField::as_atomic_field() const
{
	return this;
}

// get_num_elements returns the number of arguments (parameters) of the atomic field.
int AtomicField::get_num_elements() const
{
	return m_elements.size();
}

// get_element returns the parameter object describing the nth element.
Parameter* AtomicField::get_element(int n) const
{
	nassertr(n >= 0 && n < (int)m_elements.size(), NULL);
	return m_elements[n];
}


// output formats the field to the syntax of an atomic field in a .dc file
//     as IDENTIFIER(ELEMENTS, ...) KEYWORDS with optional ELEMENTS and KEYWORDS,
//     and outputs the formatted string to the stream.
void AtomicField::output(ostream &out, bool brief) const
{
	out << m_name << "(";

	if(!m_elements.empty())
	{
		auto it = m_elements.begin();
		output_element(out, brief, *it);
		++it;
		while(it != m_elements.end())
		{
			out << ", ";
			output_element(out, brief, *it);
			++it;
		}
	}
	out << ")";

	output_keywords(out);
}

// write generates a parseable description of the object to the indicated output stream.
void AtomicField::write(ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level);
	output(out, brief);
	out << ";";
	if(!brief && m_number >= 0)
	{
		out << "  // field " << m_number;
	}
	out << "\n";
}

// generate_hash accumulates the properties of this field into the hash
void AtomicField::generate_hash(HashGenerator &hashgen) const
{
	Field::generate_hash(hashgen);

	hashgen.add_int(m_elements.size());
	for(auto it = m_elements.begin(); it != m_elements.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}

	KeywordList::generate_hash(hashgen);
}

// get_nested_field returns the PackerInterface object that represents the nth
//     nested field.  This may return NULL if there is no such field (but it
//     shouldn't do this if n is in the range 0 <= n < get_num_nested_fields()).
PackerInterface *AtomicField::get_nested_field(int n) const
{
	nassertr(n >= 0 && n < (int)m_elements.size(), NULL);
	return m_elements[n];
}

// add_element adds a new element (parameter) to the field.
//     Normally this is called only during parsing.  The AtomicField object
//     becomes the owner of the new pointer and will delete it upon destruction.
void AtomicField::add_element(Parameter *element)
{
	m_elements.push_back(element);
	m_num_nested_fields = (int)m_elements.size();

	// See if we still have a fixed byte size.
	if(_has_fixed_byte_size)
	{
		m_has_fixed_byte_size = element->has_fixed_byte_size();
		m_fixed_byte_size += element->get_fixed_byte_size();
	}
	if(_has_fixed_structure)
	{
		m_has_fixed_structure = element->has_fixed_structure();
	}
	if(!_has_range_limits)
	{
		m_has_range_limits = element->has_range_limits();
	}
	if(!_has_default_value)
	{
		m_has_default_value = element->has_default_value();
	}
	m_default_value_stale = true;
}

// do_check_match returns true if the other interface is bitwise the same as
//     this one--that is, a uint32 only matches a uint32, etc.
//     Names of components, and range limits, are not compared.
bool AtomicField::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_atomic_field(this);
}

// do_check_match_atomic_field returns true if this field matches the
//     indicated atomic field, or false otherwise.
bool AtomicField::do_check_match_atomic_field(const AtomicField *other) const
{
	if(m_elements.size() != other->m_elements.size())
	{
		return false;
	}
	for(size_t i = 0; i < m_elements.size(); i++)
	{
		if(!m_elements[i]->check_match(other->m_elements[i]))
		{
			return false;
		}
	}

	return true;
}

// output_element formats a parameter as an element for output into .dc file syntax.
//     Used internally by AtomicField's output() method.
void AtomicField::output_element(ostream &out, bool brief, Parameter *element) const
{
	element->output(out, brief);

	if(!brief && element->has_default_value())
	{
		out << " = ";
		Packer packer;
		packer.set_unpack_data(element->get_default_value());
		packer.begin_unpack(element);
		packer.unpack_and_format(out, false);
		packer.end_unpack();
	}
}
