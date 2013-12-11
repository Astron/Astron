// Filename: ClassParameter.h
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "Parameter.h"
namespace dclass   // open namespace
{

// Forward declaration of class
class Class;

// A ClassParameter represents a class (or struct) object used as a parameter itself.
//     This means that all the fields of the class get packed into the message.
class EXPCL_DIRECT ClassParameter : public Parameter
{
	public:
		ClassParameter(const Class *dclass); // construct from class definition
		ClassParameter(const ClassParameter &copy); // copy constructor

	PUBLISHED:
		// as_class_parameter returns the same parameter pointer converted to a class parameter
		//     pointer, if this is in fact an class parameter; otherwise, returns NULL.
		virtual ClassParameter *as_class_parameter();
		virtual const ClassParameter *as_class_parameter() const;

		// make_copy returns a deep copy of this parameter
		virtual Parameter *make_copy() const;

		// is_valid returns false if the element type is an invalid type
		//     (e.g. declared from an undefined typedef), or true if it is valid.
		virtual bool is_valid() const;

		// get_class returns the class that this parameter represents
		const Class *get_class() const;

	public:
		// get_nested_field returns the PackerInterface object that represents the nth nested field.
		//     The return is NULL if 'n' is out-of-bounds of 0 <= n < get_num_nested_fields().
		virtual PackerInterface *get_nested_field(int n) const;

		// output_instance formats the parameter to the syntax of an class parameter in a .dc file
		//     as CLASS_IDENTIFIER PARAM_IDENTIFIER with optional PARAM_IDENTIFIER,
		//     and outputs the formatted string to the stream.
		virtual void output_instance(ostream &out, bool brief, const string &prename,
		                             const string &name, const string &postname) const;

		// generate_hash accumulates the properties of this type into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

	protected:
		// do_check_match returns true if the other interface is bitwise the same as
		//     this one--that is, a uint32 only matches a uint32, etc.
		//     Names of components, and range limits, are not compared.
		virtual bool do_check_match(const PackerInterface *other) const;
		virtual bool do_check_match_class_parameter(const ClassParameter *other) const;
		virtual bool do_check_match_array_parameter(const ArrayParameter *other) const;

	private:
		const Class *m_class; // class type of parameter
		std::vector<PackerInterface*> m_nested_fields; // list of nested fields in parameter
};


} // close namespace dclass
