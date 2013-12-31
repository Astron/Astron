// Filename: MolecularField.cpp
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "MolecularField.h"
#include "AtomicField.h"
#include "HashGenerator.h"
#include "indent.h"
#include <assert.h>
namespace dclass   // open namespace dclass
{


// constructor
MolecularField::MolecularField(const std::string &name, Struct *dclass) : Field(name, dclass)
{
	m_got_keywords = false;
}

// as_molecular_field returns the same field pointer converted to a molecular field pointer,
//     if this is in fact a molecular field; otherwise, returns NULL.
MolecularField *MolecularField::as_molecular_field()
{
	return this;
}

// as_molecular_field returns the same field pointer converted to a molecular field pointer,
//     if this is in fact a molecular field; otherwise, returns NULL.
const MolecularField *MolecularField::as_molecular_field() const
{
	return this;
}

// get_num_atomics returns the number of atomic fields that make up this molecular field.
int MolecularField::get_num_atomics() const
{
	return m_fields.size();
}

// get_atomic returns the nth atomic field that makes up this molecular field.
//     This may be an inherited field.
AtomicField *MolecularField::get_atomic(int n) const
{
	assert(n >= 0 && n < (int)m_fields.size());
	return m_fields[n];
}

// add_atomic adds the indicated atomic field to the end of the list of atomic fields that
//     make up the molecular field.  This is normally called only during parsing of the dc file.
//     The atomic field should be fully defined by this point; you should not modify the
//     atomic field (e.g. by adding more elements) after adding it to a molecular field.
void MolecularField::add_atomic(AtomicField *atomic)
{
	if(!m_got_keywords)
	{
		// The first atomic field determines our keywords.
		copy_keywords(*atomic);
		m_got_keywords = true;
	}
	m_fields.push_back(atomic);

	// See if we still have a fixed byte size.
	if(!atomic->has_fixed_size())
	{
		m_has_fixed_size = false;
		m_bytesize = 0;
	}
	else if(m_has_fixed_size)
	{
		m_bytesize += atomic->get_size();
	}
	/*
	if(!m_has_range_limits)
	{
		m_has_range_limits = atomic->has_range_limits();
	}
	*/
	if(!m_has_default_value)
	{
		m_has_default_value = atomic->has_default_value();
	}
	m_default_value_stale = true;
}

// output writes a representation of the field to an output stream
void MolecularField::output(std::ostream &out, bool brief) const
{
	out << m_name;

	if(!m_fields.empty())
	{
		auto field_it = m_fields.begin();
		out << " : " << (*field_it)->get_name();
		++field_it;
		while(field_it != m_fields.end())
		{
			out << ", " << (*field_it)->get_name();
			++field_it;
		}
	}

	out << ";";
}

// write writes a representation of the field to an output stream
void MolecularField::write(std::ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level);
	output(out, brief);
	if(!brief)
	{
		out << "  // field " << m_id;
	}
	out << "\n";
}

// generate_hash accumulates the properties of this field into the hash.
void MolecularField::generate_hash(HashGenerator &hashgen) const
{
	Field::generate_hash(hashgen);

	hashgen.add_int(m_fields.size());
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}


} // close namespace dclass
