// Filename: Typedef.cpp
// Created by: drose (17 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Typedef.h"
#include "Parameter.h"
#include "indent.h"
#include <sstream>
namespace dclass   // open namespace dclass
{


// constructor -- a Typedef object becomes the owner of the supplied parameter
//     pointer and will delete it upon destruction.
Typedef::Typedef(Parameter *parameter, bool implicit) :
	m_parameter(parameter), m_implicit_typedef(implicit), m_number(-1)
{
}

// destructor
Typedef::~Typedef()
{
	delete m_parameter;
}

// get_number returns a unique index number associated with this typedef definition.
int Typedef::get_number() const
{
	return m_number;
}

// get_name returns the name of this typedef.
const std::string &Typedef::get_name() const
{
	return m_parameter->get_name();
}

// get_description returns a brief decription of the typedef, useful for human consumption.
std::string Typedef::get_description() const
{
	std::ostringstream strm;
	m_parameter->output(strm, true);
	return strm.str();
}

// is_implicit_typedef returns true if the typedef has been flagged as an implicit typedef,
//     meaning it was created for a Class that was referenced inline as a type.
bool Typedef::is_implicit_typedef() const
{
	return m_implicit_typedef;
}

// new_parameter returns a newly-allocated Parameter object that uses the
//     same type as that named by the typedef.
Parameter *Typedef::new_parameter() const
{
	Parameter *new_parameter = m_parameter->copy();
	new_parameter->set_name(std::string());
	new_parameter->set_typedef(this);
	return new_parameter;
}

// set_number assigns the unique number to this typedef.
//     This is normally called only by the File interface as the typedef is added.
void Typedef::set_number(int number)
{
	m_number = number;
}

// output writes a string representation of this instance to <out>.
void Typedef::output(std::ostream &out, bool brief) const
{
	out << "typedef ";
	m_parameter->output(out, false);
}

// write writes a string representation of this instance to <out>.
void Typedef::write(std::ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level) << "typedef ";

	// We need to preserve the parameter name in the typedef (this is
	// the typedef name); hence, we pass brief = false to output().
	m_parameter->output(out, false);
	out << ";";

	if(!brief)
	{
		out << "  // typedef " << m_number;
	}
	out << "\n";
}


} // close namespace dclass