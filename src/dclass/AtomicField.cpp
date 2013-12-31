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
#include "indent.h"
#include "value/format.h"
#include <assert.h>
namespace dclass   // open namespace
{


// constructor
AtomicField::AtomicField(const std::string &name, Struct *dcc) : Field(name, dcc)
{
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
	assert(n >= 0 && n < (int)m_elements.size());
	return m_elements[n];
}


// output formats the field to the syntax of an atomic field in a .dc file
//     as IDENTIFIER(ELEMENTS, ...) KEYWORDS with optional ELEMENTS and KEYWORDS,
//     and outputs the formatted string to the stream.
void AtomicField::output(std::ostream &out, bool brief) const
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
void AtomicField::write(std::ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level);
	output(out, brief);
	out << ";";
	if(!brief && m_id >= 0)
	{
		out << "  // field " << m_id;
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

// add_element adds a new element (parameter) to the field.
//     Normally this is called only during parsing.  The AtomicField object
//     becomes the owner of the new pointer and will delete it upon destruction.
void AtomicField::add_element(Parameter *element)
{
	m_elements.push_back(element);
	if(!element->has_fixed_size())
	{
		m_has_fixed_size = false;
		m_bytesize = 0;
	}
	else if(m_has_fixed_size)
	{
		m_bytesize += element->get_size();
	}
	/*
	if(!m_has_range_limits)
	{
		m_has_range_limits = element->has_range_limits();
	}
	*/
	if(!m_has_default_value)
	{
		m_has_default_value = element->has_default_value();
	}
	m_default_value_stale = true;
}

// output_element formats a parameter as an element for output into .dc file syntax.
//     Used internally by AtomicField's output() method.
void AtomicField::output_element(std::ostream &out, bool brief, Parameter *element) const
{
	element->output(out, brief);

	if(!brief && element->has_default_value())
	{
		out << " = ";
		format(element, element->get_default_value(), out);
	}
}


} // close namespace dclass
