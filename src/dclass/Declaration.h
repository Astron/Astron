// Filename: Declaration.h
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
namespace dclass   // open namespace
{


// Forward declarations
class Class;
class Switch;

// A Declaration is a common interface for a declaration in a DC file.
//     Currently, this is either a class or a typedef declaration (import declarations are
//     still collected together at the top, and don't inherit from this object).
//     Its only purpose is so that classes and typedefs can be stored in one list
//     together so they can be ordered correctly on output.
class EXPCL_DIRECT Declaration
{
	public:
		virtual ~Declaration();

	PUBLISHED:
		// as_class returns the same declaration pointer converted to a class
		//     pointer, if this is in fact a class; otherwise, returns NULL.
		virtual Class *as_class();
		virtual const Class *as_class() const;

		// as_switch returns the same declaration pointer converted to a switch
		//     pointer, if this is in fact a switch; otherwise, returns NULL.
		virtual Switch *as_switch();
		virtual const Switch *as_switch() const;

		// output and write output representations of the declaration to an output stream
		virtual void output(ostream &out) const;
		void write(ostream &out, int indent_level) const;

	public:
		// output and write output representations of the declaration to an output stream
		virtual void output(ostream &out, bool brief) const = 0;
		virtual void write(ostream &out, bool brief, int indent_level) const = 0;
};

inline ostream &operator << (ostream &out, const Declaration &decl)
{
	decl.output(out);
	return out;
}


} // close namespace dclass
