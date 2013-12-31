// Filename: Declaration.cpp
// Created by: drose (18 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Declaration.h"
namespace dclass   // open namespace
{


// destructor
Declaration::~Declaration()
{
}

// as_struct returns the same declaration pointer converted to a class
//     pointer, if this is in fact a struct; otherwise, returns NULL.
Struct* Declaration::as_struct()
{
	return (Struct*)NULL;
}

// as_struct returns the same declaration pointer converted to a class
//     pointer, if this is in fact a struct; otherwise, returns NULL.
const Struct* Declaration::as_struct() const
{
	return (Struct*)NULL;
}

// as_class returns the same declaration pointer converted to a class
//     pointer, if this is in fact a class; otherwise, returns NULL.
Class* Declaration::as_class()
{
	return (Class*)NULL;
}

// as_class returns the same declaration pointer converted to a class
//     pointer, if this is in fact a class; otherwise, returns NULL.
const Class* Declaration::as_class() const
{
	return (Class*)NULL;
}

// output writes a string representation of the declaration to an output stream
void Declaration::output(std::ostream &out) const
{
	output(out, true);
}

// write writes a string representation of the declaration to an output stream
void Declaration::write(std::ostream &out, int indent_level) const
{
	write(out, false, indent_level);
}


} // close namespace dclass
