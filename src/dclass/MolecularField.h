// Filename: dcMolecularField.h
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "Field.h"

namespace dclass   // open namespace dclass
{


// Forward declaration
class AtomicField;
class Parameter;

// A MolecularField a single molecular field of a Distributed Class, as read from a .dc file.
//     This represents a combination of two or more related atomic fields,
//     that will often be treated as a unit.
class MolecularField : public Field
{
	public:
		MolecularField(const std::string &name, Struct *dclass);

		// as_molecular_field returns the same field pointer converted to a molecular field pointer,
		//     if this is in fact a molecular field; otherwise, returns NULL.
		virtual MolecularField *as_molecular_field();
		virtual const MolecularField *as_molecular_field() const;

		// get_num_atomics returns the number of atomic fields that make up this molecular field.
		int get_num_atomics() const;
		// get_atomic returns the nth atomic field that makes up this molecular field.
		//     This may be an inherited field.
		AtomicField *get_atomic(int n) const;

		// add_atomic adds the indicated atomic field to the end of the list of atomic
		//     fields that make up the molecular field.
		void add_atomic(AtomicField *atomic);

		// output and write write a representation of the field to an output stream
		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

	protected:
		virtual void refresh_default_value();

	private:
		// These members define the primary interface to the molecular field
		// definition as read from the file.
		std::vector<AtomicField*> m_fields;
		bool m_got_keywords;
};


} // close namespace dclass
