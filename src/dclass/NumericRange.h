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

		inline NumericRange();
		inline NumericRange(Number min, Number max);
		inline NumericRange(const NumericRange &copy);
		inline void operator = (const NumericRange &copy);

		bool is_in_range(Number num) const;
		inline void validate(Number num, bool &range_error) const;

		inline bool has_one_value() const;
		inline Number get_one_value() const;

		void generate_hash(HashGenerator &hashgen) const;

		void output(std::ostream &out, Number divisor = 1) const;
		void output_char(std::ostream &out, Number divisor = 1) const;

	public:
		inline void clear();
		bool add_range(Number min, Number max);

		inline bool is_empty() const;
		inline int get_num_ranges() const;
		inline Number get_min(int n) const;
		inline Number get_max(int n) const;

	private:
		class MinMax
		{
			public:
				inline bool operator < (const MinMax &other) const;

				Number m_min;
				Number m_max;
		};
		inline void output_minmax(std::ostream &out, Number divisor, const MinMax &range) const;
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
