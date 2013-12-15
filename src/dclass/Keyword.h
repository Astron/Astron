// Filename: Keyword.h
// Created by: drose (22 Jul, 2005)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "Declaration.h"
namespace dclass   // open namespace dclass
{


// Forward declartions
class Parameter;
class HashGenerator;

// A Keyword represents a single keyword declaration in the dc file.
//     It is used to define a communication property associated with a field,
//     for instance "broadcast" or "airecv".
class Keyword : public Declaration
{
	public:
		Keyword(const std::string &name, int historical_flag = ~0);
		virtual ~Keyword();

		// get_name returns the name of this keyword.
		const std::string &get_name() const;

		// get_historical_flag returns the bitmask associated with this keyword, if any.
		int get_historical_flag() const;
		// clear_historical_flag resets the historical flag to ~0.
		void clear_historical_flag();

		// output and write output a string representation of this instance to <out>.
		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;

		// generate_hash accumulates the properties of this keyword into the hash.
		void generate_hash(HashGenerator &hashgen) const;

	private:
		const std::string m_name;

		// This flag is only kept for historical reasons, so we can preserve
		// the file's hash code if no new flags are in use.
		int m_historical_flag;
};


} // close namespace dclass
