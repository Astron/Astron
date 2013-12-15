// Filename: Typedef.h
// Created by: drose (17 Jun, 2004)
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


// Forward declaration
class Parameter;

// A Typedef represents a single typedef declaration in the dc file.
//     It assigns a particular type to a new name, ust like a C typedef.
class Typedef : public Declaration
{
	public:
		Typedef(Parameter *parameter, bool implicit = false);
		Typedef(const std::string &name);
		virtual ~Typedef();

		int get_number() const;
		const std::string &get_name() const;
		std::string get_description() const;

		bool is_bogus_typedef() const;
		bool is_implicit_typedef() const;

		Parameter *make_new_parameter() const;

		void set_number(int number);
		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;

	private:
		Parameter *m_parameter;
		bool m_bogus_typedef;
		bool m_implicit_typedef;
		int m_number;
};


} // close namespace dclass
