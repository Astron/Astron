// Filename: AtomicField.h
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
#include "Parameter.h"
#include <math.h>
namespace dclass   // open namespace
{


// An AtomicField is a single atomic field of a Distributed Class, as read
//     from a .dc file.  This defines an interface to the Distributed Class,
//     and represents a method of the distributed class.
//     AtomicFields are most commonly used for remote procedure calls.
class AtomicField : public Field
{
	public:
		AtomicField(const std::string &name, Struct *dcc);
		virtual ~AtomicField();

		// as_atomic_field returns the same field pointer converted to an atomic field
		//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
		virtual AtomicField *as_atomic_field();
		virtual const AtomicField *as_atomic_field() const;

		// get_num_elements returns the number of arguments (parameters) of the atomic field.
		int get_num_elements() const;

		// get_element returns the parameter object describing the nth element.
		Parameter *get_element(int n) const;

		// add_element adds a new element (parameter) to the field.
		//     Normally this is called only during parsing.  The AtomicField object
		//     becomes the owner of the new pointer and will delete it upon destruction.
		void add_element(Parameter *element);

		// output formats the field to the syntax of an atomic field in a .dc file
		//     as IDENTIFIER(ELEMENTS, ...) KEYWORDS with optional ELEMENTS and KEYWORDS,
		//     and outputs the formatted string to the stream.
		virtual void output(std::ostream &out, bool brief) const;

		// write generates a parseable description of the object to the indicated output stream.
		virtual void write(std::ostream &out, bool brief, int indent_level) const;

		// generate_hash accumulates the properties of this field into the hash
		virtual void generate_hash(HashGenerator &hashgen) const;

	private:
		// output_element formats a parameter as an element for output into .dc file syntax.
		void output_element(std::ostream &out, bool brief, Parameter *element) const;

		std::vector<Parameter*> m_elements; // the "arguments" or parameters of the AtomicField
};


} // close namespace dclass
