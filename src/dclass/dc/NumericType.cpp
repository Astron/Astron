// Filename: NumericType.cpp

// This must be defined for inttypes.h to define the fixed with integer macros
#if defined(__cplusplus) && !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h> // fixed-width integer limits
#include <math.h>
#include <memory.h>
#include "util/HashGenerator.h"

#include "NumericType.h"
namespace dclass   // open namespace dclass
{


// Type constructor
NumericType::NumericType(Type type) :
    m_divisor(1), m_orig_modulus(0.0), m_orig_range()
{
    m_type = type;
    switch(type) {
    case T_CHAR:
    case T_INT8:
    case T_UINT8:
        m_size = sizeof(int8_t);
        break;
    case T_INT16:
    case T_UINT16:
        m_size = sizeof(int16_t);
        break;
    case T_INT32:
    case T_UINT32:
        m_size = sizeof(int32_t);
        break;
    case T_INT64:
    case T_UINT64:
        m_size = sizeof(int64_t);
        break;
    case T_FLOAT32:
        m_size = sizeof(float);
        break;
    case T_FLOAT64:
        m_size = sizeof(double);
        break;
    default:
        m_type = T_INVALID;
    }
}

// as_numeric returns this as a NumericType if it is numeric, or nullptr otherwise.
NumericType* NumericType::as_numeric()
{
    return this;
}
const NumericType* NumericType::as_numeric() const
{
    return this;
}

// set_divisor sets a divisor for the numeric type, typically to represent fixed-point.
//     Returns false if the divisor is not valid for this type.
bool NumericType::set_divisor(unsigned int divisor)
{
    if(divisor == 0) {
        return false;
    }

    m_divisor = divisor;

    if(has_range()) {
        set_range(m_orig_range);
    }
    if(has_modulus()) {
        set_modulus(m_orig_modulus);
    }

    return true;
}

// set_modulus sets a modulus value of the numeric type.
//     Returns false if the modulus is not valid for this type.
bool NumericType::set_modulus(double modulus)
{
    if(modulus <= 0.0) {
        return false;
    }

    double float_modulus = modulus * m_divisor;
    uint64_t uint_modulus = uint64_t(floor(modulus * m_divisor + 0.5));

    // Check the range.  A valid range for the modulus is 1 to (maximum_value + 1) after scaling.
    switch(m_type) {
    case T_CHAR:
    case T_UINT8:
        if(uint_modulus < 1 || uint16_t(UINT8_MAX) + 1 < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_UINT16:
        if(uint_modulus < 1 || uint32_t(UINT16_MAX) + 1 < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_UINT32:
        if(uint_modulus < 1 || uint64_t(UINT32_MAX) + 1L < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_UINT64:
        if(uint_modulus < 1) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_INT8:
        if(uint_modulus < 1 || uint16_t(UINT8_MAX) + 1 < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_INT16:
        if(uint_modulus < 1 || uint32_t(UINT16_MAX) + 1 < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_INT32:
        if(uint_modulus < 1 || uint64_t(UINT32_MAX) + 1L < uint_modulus) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_INT64:
        if(uint_modulus < 1) {
            return false;
        }
        m_modulus = uint_modulus;
        break;
    case T_FLOAT32:
    case T_FLOAT64:
        m_modulus = float_modulus;
        break;
    default:
        return false;
    }

    m_orig_modulus = modulus;
    return true;
}

// set_range sets a valid range of the numeric type.
//     Returns false if the range is not valid for this type.
bool NumericType::set_range(const NumericRange &range)
{
    // TODO: Accept integer ranges
    if(range.type != Number::FLOAT) {
        return false;
    }

    m_orig_range = range;
    switch(m_type) {
    case T_INT8:
    case T_INT16:
    case T_INT32:
    case T_INT64: {
        int64_t min = (int64_t)floor(range.min.floating * m_divisor + 0.5);
        int64_t max = (int64_t)floor(range.max.floating * m_divisor + 0.5);
        m_range = NumericRange(min, max);
        // TODO: Validate range, i.e. => min and max within (INT[N]_MIN - INT[N]MAX)
        break;
    }
    case T_CHAR:
    case T_UINT8:
    case T_UINT16:
    case T_UINT32:
    case T_UINT64: {
        uint64_t min = (uint64_t)floor(range.min.floating * m_divisor + 0.5);
        uint64_t max = (uint64_t)floor(range.max.floating * m_divisor + 0.5);
        m_range = NumericRange(min, max);
        // TODO: Validate range, i.e. => min and max within (UINT[N]_MIN - UINT[N]MAX)
        break;
    }
    case T_FLOAT32:
    case T_FLOAT64: {
        double min = range.min.floating * m_divisor;
        double max = range.max.floating * m_divisor;
        m_range = NumericRange(min, max);
        break;
    }
    default: {
        return false;
    }
    }

    return true;
}

std::pair<bool, Number> NumericType::data_to_number(const std::vector<uint8_t>* data) const
{
    if(m_size != data->size()) {
        return std::make_pair(false, Number(0));
    }

    switch(m_type) {
        case T_INT8: {
            int64_t val = *(int8_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_INT16: {
            int64_t val = *(int16_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_INT32: {
            int64_t val = *(int32_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_INT64: {
            return std::make_pair(true, Number(*(int64_t*)&data->front()));
        }
        case T_CHAR:
        case T_UINT8: {
            uint64_t val = *(uint8_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_UINT16: {
            uint64_t val = *(uint16_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_UINT32: {
            uint64_t val = *(uint32_t*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_UINT64: {
            return std::make_pair(true, Number(*(uint64_t*)&data->front()));
        }
        case T_FLOAT32: {
            double val = *(float*)&data->front();
            return std::make_pair(true, Number(val));
        }
        case T_FLOAT64: {
            return std::make_pair(true, Number(*(double*)&data->front()));
        }
        default: {
            break;
        }
    }

    return std::make_pair(false, Number(0));
}

bool NumericType::within_range(const std::vector<uint8_t>* data, uint64_t length) const
{
    (void) length;

    auto result = data_to_number(data);
    if(!result.first) {
        return false;
    }
  
    return m_range.contains(result.second);
}

// generate_hash accumulates the properties of this type into the hash.
void NumericType::generate_hash(HashGenerator &hashgen) const
{
    DistributedType::generate_hash(hashgen);
    hashgen.add_int(m_divisor);
    if(has_modulus()) {
        hashgen.add_int(int(m_modulus.integer));
    }
    if(has_range()) {
        hashgen.add_int(int(m_range.min.integer));
        hashgen.add_int(int(m_range.max.integer));
    }
}


} // close namespace dclass
