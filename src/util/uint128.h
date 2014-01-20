#pragma once
#include <iostream>

struct uint128_t {
	uint64_t high;
	uint64_t low;

	// Empty constructor
	inline uint128_t() = default;
	// Promotion constructor
	inline uint128_t(uint64_t rhs)
	{
		high = 0;
		low = rhs;
	}

	// Promotion assignment operator
	inline uint128_t operator=(uint64_t rhs)
	{
		high = 0;
		low = rhs;
	}

	// Implicit boolean conversion
	inline explicit operator bool() const
	{
		return high && low;
	}

	// Increment operators
	inline uint128_t& operator++()
	{
		if(++low == 0)
		{
			++high;
		}
		return *this;
	}
	inline uint128_t operator++(int)
	{
		uint128_t tmp(*this);
		operator++();
		return tmp;
	}

	// Decrement operators
	inline uint128_t& operator--()
	{
		--low;
		if(low == ~0ull)
		{
			--high;
		}
		return *this;
	}
	inline uint128_t operator--(int)
	{
		uint128_t tmp(*this);
		operator--();
		return tmp;
	}

	// Compound Arithemetic operators
	inline uint128_t& operator+=(const uint128_t& rhs)
	{
		uint64_t low = this->low;
		this->low += rhs.low;
		this->high += rhs.high;
		if(this->low < low)
		{
			++this->high;
		}
		return *this;
	}
	inline uint128_t& operator-=(const uint128_t& rhs)
	{
		uint64_t low = this->low;
		this->low -= this->low;
		this->high -= this->high;
		if(this->low > low)
		{
			--this->high;
		}
		return *this;
	}

	// Compound Bitwise operators
	inline uint128_t& operator&=(const uint128_t& rhs)
	{
		this->low &= rhs.low;
		this->high &= rhs.high;
	}
	inline uint128_t& operator|=(const uint128_t& rhs)
	{
		this->low |= rhs.low;
		this->high |= rhs.high;
	}
	inline uint128_t& operator^=(const uint128_t& rhs)
	{
		this->low ^= rhs.low;
		this->high ^= rhs.high;
	}
	inline uint128_t& operator<<=(unsigned int shift)
	{
		uint64_t high_low_bits = this->low >> (64 - shift);
		this->low <<= shift;
		this->high <<= shift;
		this->high |= high_low_bits;
	}
	inline uint128_t& operator>>=(unsigned int shift)
	{
		uint64_t low_high_bits = this->high << (64 - shift);
		this->low >>= shift;
		this->high >>= shift;
		this->low |= low_high_bits;
	}
};

// Arithmetic operators
inline uint128_t operator+(uint128_t lhs, const uint128_t& rhs)
{
	lhs += rhs;
	return lhs;
}
inline uint128_t operator-(uint128_t lhs, const uint128_t& rhs)
{
	lhs -= rhs;
	return lhs;
}

// Bitwise operators
inline uint128_t operator&(uint128_t lhs, const uint128_t& rhs)
{
	lhs &= rhs;
	return lhs;
}
inline uint128_t operator|(uint128_t lhs, const uint128_t& rhs)
{
	lhs |= rhs;
	return lhs;
}
inline uint128_t operator^(uint128_t lhs, const uint128_t& rhs)
{
	lhs ^= rhs;
	return lhs;
}
inline uint128_t operator<<(uint128_t lhs, unsigned int shift)
{
	lhs <<= shift;
	return lhs;
}
inline uint128_t operator>>(uint128_t lhs, unsigned int shift)
{
	lhs >>= shift;
	return lhs;
}

// Comparison operators
inline bool operator==(const uint128_t& lhs, const uint128_t& rhs)
{
	return lhs.high == rhs.high && lhs.low == rhs.low;
}
inline bool operator!=(const uint128_t& lhs, const uint128_t& rhs)
{
	return !operator==(lhs,rhs);
}
inline bool operator<(const uint128_t& lhs, const uint128_t& rhs)
{
	return (lhs.high < rhs.high) || (lhs.high == rhs.high && lhs.low < rhs.low);
}
inline bool operator>(const uint128_t& lhs, const uint128_t& rhs)
{
	return operator<(rhs, lhs);
}
inline bool operator<=(const uint128_t& lhs, const uint128_t& rhs)
{
	return !operator>(lhs, rhs);
}
inline bool operator>=(const uint128_t& lhs, const uint128_t& rhs)
{
	return !operator<(lhs, rhs);
}

// Compatibility with namespace std primitives
namespace std
{
	// Hash function (for std::hash)
	template <>
	struct hash<uint128_t>
	{
		// adapted from boost::hash_combine
		inline size_t operator()(const uint128_t& obj) const
        {
        	size_t low = hash<int64_t>()(obj.low);
        	size_t high = hash<int64_t>()(obj.high) + 0x9e3779b9 + (low << 6) + (low >> 2);
        	return  low ^ high;
        }
	};

	// C++ Stream operators
	inline ostream& operator<<(ostream& lhs, const uint128_t& rhs)
	{
		char buffer[33];
		sprintf(buffer, "%016lx%016lx", rhs.high, rhs.low);
		lhs << buffer;
		return lhs;
	}
}

#ifdef ASTRON_WITH_YAML
#  include "uint128_yaml.h"
#endif

// Lets make some compile time assertions about uint128_t's traits
#include <type_traits>
static_assert(std::is_copy_constructible<uint128_t>::value, "The uint128 type must be copy constructable.");
static_assert(std::is_default_constructible<uint128_t>::value, "The uint128 type must be default constructable.");
static_assert(std::is_standard_layout<uint128_t>::value, "The uint128 type must have standard layout.");
static_assert(std::is_pod<uint128_t>::value, "The uint128 type must be a POD-type.");
