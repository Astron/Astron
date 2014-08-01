// Filename: NumericRange.h
#pragma once
#include <limits> // std::numeric_limits<float>::infinity()
namespace dclass   // open namespace dclass
{


// A Number represent any C++ numeric type.
struct Number {
    enum Type {
        NONE = 0,
        INT,
        UINT,
        FLOAT,
    };

    Type type;
    union {
        int64_t integer;
        uint64_t uinteger;
        double floating;

    };

    Number()           : type(NONE),  integer(0)  {}
    Number(int32_t n)  : type(INT),   integer(n)  {}
    Number(int64_t n)  : type(INT),   integer(n)  {}
    Number(uint32_t n) : type(UINT),  uinteger(n) {}
    Number(uint64_t n) : type(UINT),  uinteger(n) {}
    Number(double n)   : type(FLOAT), floating(n) {}
    Number operator=(int64_t n)
    {
        return Number(n);
    }
    Number operator=(uint64_t n)
    {
        return Number(n);
    }
    Number operator=(double n)
    {
        return Number(n);
    }
};

inline bool operator==(const Number& a, const Number& b)
{
    return a.type == b.type && a.uinteger == b.uinteger;
}

// A NumericRange represents a range of integer or floating-point values.
//     This is used to limit numeric types; or array, string, or blob sizes.
struct NumericRange {
    Number::Type type;
    Number min;
    Number max;

    inline NumericRange() : type(Number::NONE)
    {
        min.type = max.type = Number::NONE;
        min.floating = -std::numeric_limits<float>::infinity();
        max.floating =  std::numeric_limits<float>::infinity();
    }
    inline NumericRange(int32_t min, int32_t max)   : type(Number::INT),   min(min), max(max) {}
    inline NumericRange(int64_t min, int64_t max)   : type(Number::INT),   min(min), max(max) {}
    inline NumericRange(uint32_t min, uint32_t max) : type(Number::UINT),  min(min), max(max) {}
    inline NumericRange(uint64_t min, uint64_t max) : type(Number::UINT),  min(min), max(max) {}
    inline NumericRange(double min, double max)     : type(Number::FLOAT), min(min), max(max) {}

    inline bool contains(Number num) const
    {
        switch(min.type) {
        case Number::NONE:
            return true;
        case Number::INT:
            return (min.integer <= num.integer && num.integer <= max.integer);
        case Number::UINT:
            return (min.uinteger <= num.uinteger && num.uinteger <= max.uinteger);
        case Number::FLOAT:
            return (min.floating <= num.floating && num.floating <= max.floating);
        }
    }

    inline bool is_empty() const
    {
        return (type == Number::NONE);
    }
};


} // close namespace dclass
