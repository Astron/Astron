// Filename: NumericRange.h
#pragma once
#include <limits>
namespace dclass   // open namespace dclass
{

enum NumericType
{
	NT_none = 0,
	NT_uint,
	NT_sint,
	NT_float,
};

// A Number represent any C++ numeric type.
struct Number {
	NumericType type;
	union
	{
		int64_t sinteger;
		uint64_t uinteger;
		double floating;

	};

	Number()           : type(NT_none),  uinteger(0) {}
	Number(int32_t n)  : type(NT_sint),  sinteger(n) {}
	Number(int64_t n)  : type(NT_sint),  sinteger(n) {}
	Number(uint32_t n) : type(NT_uint),  uinteger(n) {}
	Number(uint64_t n) : type(NT_uint),  uinteger(n) {}
	Number(double n)   : type(NT_float), floating(n) {}
};

inline bool operator==(const Number& a, const Number& b)
{
	return a.type == b.type && a.uinteger == b.uinteger;
}

// A NumericRange represents a range of integer or floating-point values.
//     This is used to limit numeric types; or array, string, or blob sizes.
struct NumericRange
{
	NumericType type;
	Number min;
	Number max;

	inline NumericRange() : type(NT_none)
	{
		min.type = max.type = NT_none;
		min.floating = -std::numeric_limits<double>::infinity();
		max.floating =  std::numeric_limits<double>::infinity();
	}
	inline NumericRange(int32_t min, int32_t max)   : type(NT_sint),  min(min), max(max) {}
	inline NumericRange(int64_t min, int64_t max)   : type(NT_sint),  min(min), max(max) {}
	inline NumericRange(uint32_t min, uint32_t max) : type(NT_uint),  min(min), max(max) {}
	inline NumericRange(uint64_t min, uint64_t max) : type(NT_uint),  min(min), max(max) {}
	inline NumericRange(double min, double max)     : type(NT_float), min(min), max(max) {}

	inline bool contains(Number num) const
	{
		switch(min.type)
		{
			case NT_none:
				return true;
			case NT_uint:
				return (min.uinteger <= num.uinteger && num.uinteger <= max.uinteger);
			case NT_sint:
				return (min.sinteger <= num.sinteger && num.sinteger <= max.sinteger);
			case NT_float:
				return (min.floating <= num.floating && num.floating <= max.floating);
		}
	}

	inline bool is_empty() const
	{
		return (type == NT_none);
	}
};


} // close namespace dclass
