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

// A Declaration is a common interface for a declaration in a DC file.
//     This is either a class, typedef, or keyword declaration.
class EXPCL_DIRECT Declaration
{
	public:
		virtual ~Declaration();

	PUBLISHED:
		// as_class returns the same declaration pointer converted to a class
		//     pointer, if this is in fact a class; otherwise, returns NULL.
		virtual Class *as_class();
		virtual const Class *as_class() const;

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
