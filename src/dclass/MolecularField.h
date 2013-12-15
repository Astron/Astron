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
#include "dcbase.h"
#include "Field.h"
namespace dclass   // open namespace dclass
{


// Forward declaration
class AtomicField;
class Parameter;

// A MolecularField a single molecular field of a Distributed Class, as read from a .dc file.
//     This represents a combination of two or more related atomic fields,
//     that will often be treated as a unit.
class EXPCL_DIRECT MolecularField : public Field
{
	public:
		MolecularField(const std::string &name, Class *dclass);

	PUBLISHED:
		// as_molecular_field returns the same field pointer converted to a molecular field pointer,
		//     if this is in fact a molecular field; otherwise, returns NULL.
		virtual MolecularField *as_molecular_field();
		virtual const MolecularField *as_molecular_field() const;

		// get_num_atomics returns the number of atomic fields that make up this molecular field.
		int get_num_atomics() const;
		// get_atomic returns the nth atomic field that makes up this molecular field.
		//     This may be an inherited field.
		AtomicField *get_atomic(int n) const;

	public:

		// add_atomic adds the indicated atomic field to the end of the list of atomic
		//     fields that make up the molecular field.
		void add_atomic(AtomicField *atomic);

		// output and write write a representation of the field to an output stream
		virtual void output(ostream &out, bool brief) const;
		virtual void write(ostream &out, bool brief, int indent_level) const;

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

		// get_nested_field returns the PackerInterface object that represents the nth nested field.
		//     Will return null if the field is out of the range 0 <= n < get_num_nested_fields().
		virtual PackerInterface *get_nested_field(int n) const;

	protected:
		// do_check_match returns true if the other interface is bitwise the same as
		//     this one that is, a uint32 only matches a uint32, etc.
		//     Names of components, and range limits, are not compared.
		virtual bool do_check_match(const PackerInterface *other) const;
		virtual bool do_check_match_molecular_field(const MolecularField *other) const;

	private:
		// These members define the primary interface to the molecular field
		// definition as read from the file.
		std::vector<AtomicField*> m_fields;
		bool m_got_keywords;

		Parameter* get_next_pack_element();

		std::vector<PackerInterface*> m_nested_fields;
};


} // close namespace dclass
