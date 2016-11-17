// Filename: ArrayType.h
#pragma once
#include <stddef.h> // size_t
#include "NumericRange.h"

#include "DistributedType.h"
namespace dclass   // open namespace
{


// An ArrayType represents an array of some other kind of object, meaning
//     this parameter type accepts an arbitrary (or possibly fixed) number of
//     nested fields, all of which are of the same type.
//     Strings and blobs are arrays with char and uint8 elements respectively.
class ArrayType : public DistributedType
{
  public:
    ArrayType(DistributedType* element_type, const NumericRange &size = NumericRange());

    // as_array returns this as a ArrayType if it is an array/string/blob, or nullptr otherwise.
    virtual ArrayType* as_array();
    virtual const ArrayType* as_array() const;

    // get_element_type returns the type of the individual elements of this array.
    inline DistributedType* get_element_type();
    inline const DistributedType* get_element_type() const;

    // get_array_size returns the fixed number of elements in this array,
    //     or 0 if the array may contain a variable number of elements.
    inline size_t get_array_size() const;

    // has_range returns true if there is a constraint on the range of valid array sizes.
    //     This is always true for fixed-size arrays.
    inline bool has_range() const;
    // get_range returns the range of sizes that the array may have.
    inline NumericRange get_range() const;

    // generate_hash accumulates the properties of this type into the hash.
    virtual void generate_hash(HashGenerator &hashgen) const;

  private:
    DistributedType *m_element_type; // type of the elements contained in the array
    unsigned int m_array_size; // number of elements in the array if it is a constant (or 0)
    NumericRange m_array_range; // the range of possible elements in the array
};


} // close namespace dclass
#include "ArrayType.ipp"
