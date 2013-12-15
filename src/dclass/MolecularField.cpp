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
namespace dclass   // open namespace dclass
{


// constructor
MolecularField::MolecularField(const std::string &name, Class *dclass) : Field(name, dclass)
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
	nassertr(n >= 0 && n < (int)m_fields.size(), NULL);
	return m_fields[n];
}

// add_atomic adds the indicated atomic field to the end of the list of atomic fields that
//     make up the molecular field.  This is normally called only during parsing of the dc file.
//     The atomic field should be fully defined by this point; you should not modify the
//     atomic field (e.g. by adding more elements) after adding it to a molecular field.
void MolecularField::add_atomic(AtomicField *atomic)
{
	if(!atomic->is_bogus_field())
	{
		if(!m_got_keywords)
		{
			// The first non-bogus atomic field determines our keywords.
			copy_keywords(*atomic);
			m_got_keywords = true;
		}
	}
	m_fields.push_back(atomic);

	int num_atomic_fields = atomic->get_num_nested_fields();
	for(int i = 0; i < num_atomic_fields; i++)
	{
		m_nested_fields.push_back(atomic->get_nested_field(i));
	}

	m_num_nested_fields = m_nested_fields.size();

	// See if we still have a fixed byte size.
	if(m_has_fixed_byte_size)
	{
		m_has_fixed_byte_size = atomic->has_fixed_byte_size();
		m_fixed_byte_size += atomic->get_fixed_byte_size();
	}
	if(m_has_fixed_structure)
	{
		m_has_fixed_structure = atomic->has_fixed_structure();
	}
	if(!m_has_range_limits)
	{
		m_has_range_limits = atomic->has_range_limits();
	}
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
		out << "  // field " << m_number;
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

// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     Will return null if the field is out of the range 0 <= n < get_num_nested_fields().
PackerInterface *MolecularField::get_nested_field(int n) const
{
	nassertr(n >= 0 && n < (int)m_nested_fields.size(), NULL);
	return m_nested_fields[n];
}

// do_check_match returns true if the other interface is bitwise the same as
//     this one that is, a uint32 only matches a uint32, etc.
//     Names of components, and range limits, are not compared.
bool MolecularField::do_check_match(const PackerInterface *other) const
{
	return other->do_check_match_molecular_field(this);
}

// do_check_match_molecular_field returns true if this field matches the indicated molecular field.
bool MolecularField::do_check_match_molecular_field(const MolecularField *other) const
{
	if(m_nested_fields.size() != other->m_nested_fields.size())
	{
		return false;
	}
	for(size_t i = 0; i < m_nested_fields.size(); i++)
	{
		if(!m_nested_fields[i]->check_match(other->m_nested_fields[i]))
		{
			return false;
		}
	}

	return true;
}


} // close namespace dclass
