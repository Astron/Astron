// Filename: StructParameter.h
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "Parameter.h"
#include "Struct.h"
namespace dclass   // open namespace
{


// A StructParameter represents a struct (or class) object used as a parameter itself.
//     This means that all the fields of the class get packed into the message.
class StructParameter : public Parameter
{
	public:
		StructParameter(const Struct *dclass); // construct from class definition
		StructParameter(const StructParameter &copy); // copy constructor

		// as_struct_parameter returns the same parameter pointer converted to a struct parameter
		//     pointer, if this is in fact an struct parameter; otherwise, returns NULL.
		virtual StructParameter *as_struct_parameter();
		virtual const StructParameter *as_struct_parameter() const;

		// make_copy returns a deep copy of this parameter
		virtual Parameter *make_copy() const;

		// is_valid returns false if the element type is an invalid type
		//     (e.g. declared from an undefined typedef), or true if it is valid.
		virtual bool is_valid() const;

		// get_class returns the class that this parameter represents
		const Struct *get_class() const;

		// output_instance formats the parameter to the syntax of an class parameter in a .dc file
		//     as CLASS_IDENTIFIER PARAM_IDENTIFIER with optional PARAM_IDENTIFIER,
		//     and outputs the formatted string to the stream.
		virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                             const std::string &name, const std::string &postname) const;

		// generate_hash accumulates the properties of this type into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

	private:
		const Struct *m_class; // class type of parameter
};


} // close namespace dclass
