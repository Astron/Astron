// Filename: SimpleParameter.h
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
#include "Parameter.h"
#include "NumericRange.h"
namespace dclass   // open namespace dclass
{


// A SimpleParameter is the most fundamental kind of parameter type: a single number or string,
//     one of the SubatomicType elements.  It may also optionally have a divisor, which is
//     meaningful only for the numeric type elements (and represents a fixed-point numeric convention).
class SimpleParameter : public Parameter
{
	public:
		// Type constructor
		SimpleParameter(DataType type, unsigned int divisor = 1);
		// Copy constructor
		SimpleParameter(const SimpleParameter &copy);

		// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
		//     if this is in fact a simple parameter; otherwise, returns NULL.
		virtual SimpleParameter *as_simple_parameter();
		virtual const SimpleParameter *as_simple_parameter() const;

		virtual Parameter *make_copy() const;
		virtual bool is_valid() const;

		bool has_modulus() const;
		double get_modulus() const;
		int get_divisor() const;

		bool is_numeric_type() const;
		bool set_modulus(double modulus);
		bool set_divisor(unsigned int divisor);
		bool set_range(const NumericRange &range);

		virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                             const std::string &name, const std::string &postname) const;
		virtual void generate_hash(HashGenerator &hashgen) const;

	private:
		unsigned int m_divisor;

		// These are the original range and modulus values from the file, unscaled by the divisor.
		bool m_has_modulus;
		double m_orig_modulus;
		NumericRange m_orig_range;

		// These are the range and modulus values after scaling by the divisor.
		Number m_modulus;
		NumericRange m_range;
};


} // close namespace dclass
