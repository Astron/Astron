// Filename: Parameter.h
// Created by: drose (15 Jun, 2004)
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
#include "NumericRange.h"
namespace dclass   // open namespace dclass
{


// Foward declarations
class SimpleParameter;
class ClassParameter;
class ArrayParameter;
class Typedef;
class HashGenerator;

// A Parameter represents the type specification for a single parameter within a field specification.
//     This may be a simple type, or it may be a class or an array reference.
//     This may also be a typedef reference to another type, which has the same properties as
//     the referenced type, but a different name.
class Parameter : public Field
{
	protected:
		// null constructor
		Parameter();
		// copy constructor
		Parameter(const Parameter &copy);
	public:
		virtual ~Parameter();

		// as_parameter returns the same field pointer converted to a parameter,
		//     if this is in fact a parameter; otherwise, returns NULL.
		virtual Parameter *as_parameter();
		virtual const Parameter *as_parameter() const;

		// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
		//     if this is in fact a simple parameter; otherwise, returns NULL.
		virtual SimpleParameter *as_simple_parameter();
		virtual const SimpleParameter *as_simple_parameter() const;

		// as_class_parameter returns the same parameter pointer converted to a class parameter,
		//     if this is in fact a class parameter; otherwise, returns NULL.
		virtual ClassParameter *as_class_parameter();
		virtual const ClassParameter *as_class_parameter() const;

		// as_array_parameter returns the same parameter pointer converted to an array parameter,
		//     if this is in fact an array parameter; otherwise, returns NULL.
		virtual ArrayParameter *as_array_parameter();
		virtual const ArrayParameter *as_array_parameter() const;

		virtual Parameter *make_copy() const = 0;
		virtual bool is_valid() const = 0;

		// get_typedef returns the Typedef instance if this type has been referenced from a typedef, or NULL.
		const Typedef *get_typedef() const;

		// set_typedef records the Typedef object that generated this parameter.
		//     This is normally called only from Typedef::make_new_parameter().
		void set_typedef(const Typedef *dtypedef);

		// append_array_specification returns the type represented by this_type[size].
		//     In the case of a generic Parameter, it returns an ArrayParameter wrapped around this type.
		virtual Parameter *append_array_specification(const UnsignedIntRange &size);

		// output and write write a representation of the parameter to an output stream.
		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;

		// output_instance and write_instance format the parameter in .dc file syntax.
		virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                             const std::string &name, const std::string &postname) const = 0;
		virtual void write_instance(std::ostream &out, bool brief, int indent_level,
		                            const std::string &prename, const std::string &name,
		                            const std::string &postname) const;

		// output_typedef_name formats the instance like output_instance, but uses the typedef name instead.
		void output_typedef_name(std::ostream &out, bool brief, const std::string &prename,
		                         const std::string &name, const std::string &postname) const;
		void write_typedef_name(std::ostream &out, bool brief, int indent_level,
		                        const std::string &prename, const std::string &name,
		                        const std::string &postname) const;

		// generate_hash accumulates the properties of this type into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

	private:
		const Typedef *m_typedef;
};


} // close namespace dclass
