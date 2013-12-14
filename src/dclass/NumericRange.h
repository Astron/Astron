// Filename: NumericRange.h
// Created by: drose (21 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "HashGenerator.h"
#include "Packer.h"
namespace dclass   // open namespace dclass
{


// A NumericRange represents a range of legal integer or floating-point values.
//     This is used to constrain simple numeric types, as well as array sizes.
template <class NUM>
class NumericRange
{
	public:
		typedef NUM Number;

		// empty constructor
		inline NumericRange();
		// single-range constructor
		inline NumericRange(Number min, Number max);
		// copy constructor
		inline NumericRange(const NumericRange &copy);
		// copy assignment operator
		inline void operator = (const NumericRange &copy);

		// is_in_range returns true if the indicated number is within the specified range, false otherwise.
		bool is_in_range(Number num) const;
		// validate sets <range_error> to true if <num> is outside of the range.
		inline void validate(Number num, bool &range_error) const;

		// has_one_value returns true if the numeric range specifies exactly one legal value.
		inline bool has_one_value() const;
		// get_one_value returns the value accepted by the numeric range, if there is only one legal value.
		inline Number get_one_value() const;

		// generate_hash accumulates the properties of this range into the hash.
		void generate_hash(HashGenerator &hashgen) const;

		// output writes a representation of the range to an output stream
		void output(std::ostream &out, Number divisor = 1) const;
		// output_char outputs the range, formatting the numeric values as quoted ASCII characters.
		void output_char(std::ostream &out, Number divisor = 1) const;

	public:
		// clear reset the range to an empty range
		inline void clear();
		// add_range adds a new minmax to the list of ranges.  This is normally called only during parsing.
		//     Returns true if successful, or false if the new minmax overlaps an existing minmax.
		bool add_range(Number min, Number max);

		// is_empty returns true if the range contains no elements (and thus allows all numbers),
		//     false if it contains at least one.
		inline bool is_empty() const;
		// get_num_ranges returns the number of minmax components in the range description.
		inline int get_num_ranges() const;
		// get_min returns the minimum value defined by the nth component.
		inline Number get_min(int n) const;
		// get_max returns the maximum value defined by the nth component.
		inline Number get_max(int n) const;

	private:
		class MinMax
		{
			public:
				inline bool operator < (const MinMax &other) const;

				Number m_min;
				Number m_max;
		};

		// output_minmax outputs a single element of the range description.
		inline void output_minmax(std::ostream &out, Number divisor, const MinMax &range) const;
		// output_minmax_char outputs a single element of the range description.
		inline void output_minmax_char(std::ostream &out, const MinMax &range) const;

		std::vector<MinMax> m_ranges;
};

typedef NumericRange<int> IntRange;
typedef NumericRange<unsigned int> UnsignedIntRange;
typedef NumericRange<int64_t> Int64Range;
typedef NumericRange<uint64_t> UnsignedInt64Range;
typedef NumericRange<double> DoubleRange;


} // close namespace dclass

#include "NumericRange.ipp"
