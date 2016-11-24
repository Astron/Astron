// Filename: ArrayType.cpp

// This must be defined for inttypes.h to define the fixed with integer macros
#if defined(__cplusplus) && !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include "util/HashGenerator.h"

#include "ArrayType.h"
namespace dclass   // open namespace
{


// type constructor
ArrayType::ArrayType(DistributedType* element_type, const NumericRange& size) :
    m_element_type(element_type), m_array_range(size)
{
    if(m_element_type == nullptr) {
        m_element_type = DistributedType::invalid;
    }

    // TODO: Handle non-uinteger NumericRanges
    if(m_array_range.is_empty()) {
        m_array_size = 0;
        m_array_range.min.uinteger = 0;
        m_array_range.max.uinteger = UINT64_MAX;
    } else if(m_array_range.min == m_array_range.max) {
        m_array_size = (unsigned int)m_array_range.min.uinteger;
    } else {
        m_array_size = 0;
    }

    if(m_element_type->has_fixed_size() && m_array_size > 0) {
        m_type = T_ARRAY;
        m_size = m_array_size * m_element_type->get_size();
    } else {
        m_type = T_VARARRAY;
        m_size = 0;
    }



    if(m_element_type->get_type() == T_CHAR) {
        if(m_type == T_ARRAY) {
            m_type = T_STRING;
        } else {
            m_type = T_VARSTRING;
        }
    } else if(m_element_type->get_type() == T_UINT8) {
        if(m_type == T_ARRAY) {
            m_type = T_BLOB;
        } else {
            m_type = T_VARBLOB;
        }
    }
}

// as_array returns this as an ArrayType if it is an array, or nullptr otherwise.
ArrayType* ArrayType::as_array()
{
    return this;
}
const ArrayType* ArrayType::as_array() const
{
    return this;
}

// generate_hash accumulates the properties of this type into the hash.
void ArrayType::generate_hash(HashGenerator& hashgen) const
{
    DistributedType::generate_hash(hashgen);
    m_element_type->generate_hash(hashgen);
    if(has_range()) {
        hashgen.add_int(int(m_array_range.min.integer));
        hashgen.add_int(int(m_array_range.max.integer));
    } else {
        hashgen.add_int(m_array_size);
    }
}


} // close namespace dclass
