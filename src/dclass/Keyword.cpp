// Filename: Keyword.cpp
// Created by: drose (22 Jul, 2005)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Keyword.h"
#include "HashGenerator.h"
#include "indent.h"
namespace dclass   // open namespace dclass
{


// constructor
Keyword::Keyword(const std::string &name, int historical_flag) :
	m_name(name), m_historical_flag(historical_flag)
{
}

// destructor
Keyword::~Keyword()
{
}

// get_name returns the name of this keyword.
const std::string &Keyword::get_name() const
{
	return m_name;
}

// get_historical_flag returns the bitmask associated with this keyword, if any.
//     This is the value that was historically associated with this keyword, and was used to
//     generate a hash code before we had user-customizable keywords.
//     It will return ~0 if this is not an historical keyword.
int Keyword::get_historical_flag() const
{
	return m_historical_flag;
}

// clear_historical_flag resets the historical flag to ~0,
//     as if the keyword were not one of the historically defined keywords.
void Keyword::clear_historical_flag()
{
	m_historical_flag = ~0;
}

// output writes a string representation of this instance to <out>.
void Keyword::output(ostream &out, bool brief) const
{
	out << "keyword " << m_name;
}

// write writes a string representation of this instance to <out>.
void Keyword::write(ostream &out, bool, int indent_level) const
{
	indent(out, indent_level) << "keyword " << m_name << ";\n";
}

// generate_hash accumulates the properties of this keyword into the hash.
void Keyword::generate_hash(HashGenerator &hashgen) const
{
	hashgen.add_string(m_name);
}


} // close namespace dclass
