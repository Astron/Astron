// Filename: ArrayParameter.h
// Created by: drose (17 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "Parameter.h"
#include "NumericRange.h"
#include <sstream> // for std::ostringstream
namespace dclass   // open namespace
{


// An ArrayParameter represents an array of some other kind of object, meaning
//     this parameter type accepts an arbitrary (or possibly fixed) number of
//     nested fields, all of which are of the same type.
class ArrayParameter : public Parameter
{
	public:
		// basic constructor
		ArrayParameter(Parameter *element_type, const NumericRange &size = NumericRange());
		// copy constructor
		ArrayParameter(const ArrayParameter &copy);

		virtual ~ArrayParameter();

		// as_array_parameter returns the same parameter pointer converted to an array parameter
		//     pointer, if this is in fact an array parameter; otherwise, returns NULL.
		virtual ArrayParameter* as_array_parameter();
		virtual const ArrayParameter* as_array_parameter() const;

		// copy returns a deep copy of this ArrayParameter
		Parameter* copy() const;

		// get_element_type returns the type of the individual elements of this array.
		Parameter* get_element_type() const;

		// get_array_size returns the fixed number of elements in this array,
		//     or -1 if the array may contain a variable number of elements.
		int get_array_size() const;

		// append_array_specification returns the type represented by this type[size].
		//     As an ArrayParameter, this same pointer is returned, but the inner type of the array
		//     becomes an array type (ie. type[] becomes type[][]).
		virtual Parameter* append_array_specification(const NumericRange &size);

		// output_instance formats the parameter to the syntax of an array parameter in a .dc file
		//     as TYPE IDENTIFIER[RANGE] with optional IDENTIFIER and RANGE,
		//     and outputs the formatted string to the stream.
		virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                             const std::string &name, const std::string &postname) const;

		// generate_hash accumulates the properties of this type into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

	protected:
		virtual void refresh_default_value();

	private:
		Parameter *m_element_type; // type of the elements contained in the array
		unsigned int m_array_size; // number of elements in the array if it is a constant (or 0)
		NumericRange m_array_range; // the range of possible elements in the array
};


} // close namespace dclass
