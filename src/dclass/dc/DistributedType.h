// Filename: DistributedType.h
#pragma once
#include <stdint.h>
#include <string> // std::string
namespace dclass   // open namespace dclass
{


#ifdef DCLASS_32BIT_SIZETAG
typedef uint32_t sizetag_t;
#else
typedef uint16_t sizetag_t;
#endif

// Forward declaration
class ArrayType;
class Method;
class NumericType;
class Struct;
class HashGenerator;


// The Type enum are numeric constants representing the layout of the DistributedType
enum Type {
    /* Numeric Types */
    T_INT8, T_INT16, T_INT32, T_INT64,
    T_UINT8, T_UINT16, T_UINT32, T_UINT64,
    T_CHAR, // equivalent to uint8, except that it should be printed as a string
    T_FLOAT32, T_FLOAT64,

    /* Array Types */
    T_STRING,      // a human-printable string with fixed length
    T_VARSTRING,   // a human-printable string with variable length
    T_BLOB,        // any binary data stored as a string, fixed length
    T_VARBLOB,     // any binary data stored as a varstring, variable length
    T_ARRAY,       // any array with fixed byte-length (fixed array-size and element-length)
    T_VARARRAY,    // any array with variable array-size or variable length elements

    /* Complex Types */
    T_STRUCT,
    T_METHOD,

    // New additions should be added at the end to prevent the file hash from changing.

    T_INVALID
};

// A DistributedType is a shared type with a defined layout of data.
class DistributedType
{
  protected:
    inline DistributedType();

  public:
    static DistributedType* invalid;
    virtual ~DistributedType();

    // get_type returns the type's fundamental type as an integer constant.
    inline Type get_type() const;

    // has_fixed_size returns true if the DistributedType has a fixed size in bytes.
    inline bool has_fixed_size() const;
    // get_size returns the size of the DistributedType in bytes or 0 if it is variable.
    inline sizetag_t get_size() const;

    // has_alias returns true if this type was defined the an aliased name.
    inline bool has_alias() const;
    // get_alias returns the name used to define the type, or the empty string.
    inline const std::string& get_alias() const;
    // set_alias gives this type the alternate name <alias>.
    inline void set_alias(const std::string& alias);

    // as_numeric returns this as a NumericType if it is numeric, or nullptr otherwise.
    virtual NumericType* as_numeric();
    virtual const NumericType* as_numeric() const;

    // as_array returns this as an ArrayType if it is an array, or nullptr otherwise.
    virtual ArrayType* as_array();
    virtual const ArrayType* as_array() const;

    // as_struct returns this as a Struct if it is a struct, or nullptr otherwise.
    virtual Struct* as_struct();
    virtual const Struct* as_struct() const;

    // as_method returns this as a Method if it is a method, or nullptr otherwise.
    virtual Method* as_method();
    virtual const Method* as_method() const;

    // generate_hash accumulates the properties of this file into the hash.
    virtual void generate_hash(HashGenerator& hashgen) const;

  protected:
    Type m_type;
    sizetag_t m_size;
    std::string m_alias;
};


} // close namespace dclass
#include "DistributedType.ipp"
