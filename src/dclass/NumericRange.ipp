// Filename: NumericRange.ipp
// Created by: drose (21 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
namespace dclass   // open namespace dclass
{


// null constructor
template <class NUM>
inline NumericRange<NUM>::NumericRange()
{
}

// range constructor
template <class NUM>
inline NumericRange<NUM>::NumericRange(Number min, Number max)
{
	add_range(min, max);
}

// copy constructor
template <class NUM>
inline NumericRange<NUM>::NumericRange(const NumericRange<NUM> &copy) : m_ranges(copy.m_ranges)
{
}

// copy assignment operator
template <class NUM>
inline void NumericRange<NUM>::operator = (const NumericRange<NUM> &copy)
{
	m_ranges = copy.m_ranges;
}

// is_in_range returns true if the indicated number is within the specified range, false otherwise.
template <class NUM>
bool NumericRange<NUM>::is_in_range(Number num) const
{
	if(m_ranges.empty())
	{
		return true;
	}

	for(auto it = m_ranges.begin(); it != m_ranges.end(); ++it)
	{
		if(num >= it->m_min && num <= it->m_max)
		{
			return true;
		}
	}

	return false;
}

// validate sets <range_error> to true if <num> is outside of the range.
template <class NUM>
inline void NumericRange<NUM>::validate(Number num, bool &range_error) const
{
	if(!is_in_range(num))
	{
		range_error = true;
	}
}

// has_one_value returns true if the numeric range specifies exactly one legal value.
template <class NUM>
inline bool NumericRange<NUM>::has_one_value() const
{
	return m_ranges.size() == 1 && m_ranges[0].m_min == m_ranges[0].m_max;
}

// get_one_value returns the value accepted by the numeric range, if there is only one legal value.
template <class NUM>
inline typename NumericRange<NUM>::Number NumericRange<NUM>::get_one_value() const
{
	assert(has_one_value());
	return m_ranges[0].m_min;
}

// generate_hash accumulates the properties of this range into the hash.
template <class NUM>
void NumericRange<NUM>::generate_hash(HashGenerator &hashgen) const
{
	if(!m_ranges.empty())
	{
		hashgen.add_int(m_ranges.size());
		for(auto it = m_ranges.begin(); it != m_ranges.end(); ++it)
		{
			// We don't account for the fractional part of floating-point
			// ranges here.  Shouldn't be a real issue.
			hashgen.add_int((int)it->m_min);
			hashgen.add_int((int)it->m_max);
		}
	}
}

// output writes a representation of the range to an output stream
template <class NUM>
void NumericRange<NUM>::output(std::ostream &out, Number divisor) const
{
	if(!m_ranges.empty())
	{
		auto range_it = m_ranges.begin();
		output_minmax(out, divisor, *range_it);
		++range_it;
		while(range_it != m_ranges.end())
		{
			out << ", ";
			output_minmax(out, divisor, *range_it);
			++range_it;
		}
	}
}

// output_char outputs the range, formatting the numeric values as quoted ASCII characters.
template <class NUM>
void NumericRange<NUM>::output_char(std::ostream &out, Number divisor) const
{
	if(divisor != 1)
	{
		output(out, divisor);
	}
	else
	{
		if(!m_ranges.empty())
		{
			auto range_it = m_ranges.begin();
			output_minmax_char(out, *range_it);
			++range_it;
			while(range_it != m_ranges.end())
			{
				out << ", ";
				output_minmax_char(out, *range_it);
				++range_it;
			}
		}
	}
}

// clear reset the range to an empty range
template <class NUM>
inline void NumericRange<NUM>::clear()
{
	m_ranges.clear();
}

// add_range adds a new minmax to the list of ranges.  This is normally called only during parsing.
//     Returns true if successful, or false if the new minmax overlaps an existing minmax.
template <class NUM>
bool NumericRange<NUM>::add_range(Number min, Number max)
{
	// Check for an overlap.  This is probably indicative of a typo and
	// should be reported.
	if(max < min)
	{
		return false;
	}

	for(auto it = m_ranges.begin(); it != m_ranges.end(); ++it)
	{
		if((min >= it->m_min && min <= it->m_max) ||
		        (max >= it->m_min && max <= it->m_max) ||
		        (min < it->m_min && max > it->m_max))
		{
			return false;
		}
	}

	MinMax minmax;
	minmax.m_min = min;
	minmax.m_max = max;
	m_ranges.push_back(minmax);

	return true;
}

// is_empty returns true if the range contains no elements (and thus allows all numbers),
//     false if it contains at least one.
template <class NUM>
inline bool NumericRange<NUM>::is_empty() const
{
	return m_ranges.empty();
}

// get_num_ranges returns the number of minmax components in the range description.
template <class NUM>
inline int NumericRange<NUM>::get_num_ranges() const
{
	return m_ranges.size();
}

// get_min returns the minimum value defined by the nth component.
template <class NUM>
inline typename NumericRange<NUM>::Number NumericRange<NUM>::get_min(int n) const
{
	assert(n >= 0 && n < (int)m_ranges.size());
	return m_ranges[n].m_min;
}

// get_max returns the maximum value defined by the nth component.
template <class NUM>
inline typename NumericRange<NUM>::Number NumericRange<NUM>::get_max(int n) const
{
	assert(n >= 0 && n < (int)m_ranges.size());
	return m_ranges[n].m_max;
}

// output_minmax outputs a single element of the range description.
template <class NUM>
inline void NumericRange<NUM>::output_minmax(std::ostream &out, Number divisor, const MinMax &range) const
{
	if(divisor == 1)
	{
		if(range.m_min == range.m_max)
		{
			out << range.m_min;
		}
		else
		{
			out << range.m_min << "-" << range.m_max;
		}
	}
	else
	{
		if(range.m_min == range.m_max)
		{
			out << (double)range.m_min / (double)divisor;
		}
		else
		{
			out << (double)range.m_min / (double)divisor
			    << "-"
			    << (double)range.m_max / (double)divisor;
		}
	}
}

// output_minmax_char outputs a single element of the range description.
template <class NUM>
inline void NumericRange<NUM>::output_minmax_char(std::ostream &out, const MinMax &range) const
{
	if(range.m_min == range.m_max)
	{
		Packer::enquote_string(out, '\'', std::string(1, range.m_min));
	}
	else
	{
		Packer::enquote_string(out, '\'', std::string(1, range.m_min));
		out << "-";
		Packer::enquote_string(out, '\'', std::string(1, range.m_max));
	}
}


} // close namespace dclass
