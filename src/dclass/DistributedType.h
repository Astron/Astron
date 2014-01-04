// Filename: DistributedType.h
#pragma once
namespace dclass   // open namespace dclass
{


#ifdef DCLASS_32BIT_SIZETAG
    typedef uint32_t sizetag_t;
#else
    typedef uint16_t sizetag_t;
#endif

// Forward declaration
class ArrayType
class MethodType;
class NumericType;
class StructType;
class HashGenerator;

// A DistributedType is a shared type with a defined layout of data.
class DistributedType
{
    protected:
        inline DistributedType();
        inline DistributedType(const DistributedType& copy);

    public:
        enum Type
        {
            /* Numeric Types */
            CHAR,           // equivalent to uint8, except that it should be printed as a string
            INT8, INT16, INT32, INT64,
            UINT8, UINT16, UINT32, UINT64,
            FLOAT32, FLOAT64

            /* Array Types */
            STRING,      // a human-printable string with fixed length
            VARSTRING,   // a human-printable string with variable length
            BLOB,        // any binary data stored as a string, fixed length
            VARBLOB,     // any binary data stored as a varstring, variable length
            ARRAY,       // any array with fixed byte-length (fixed array-size and element-length)
            VARARRAY,    // any array with variable array-size or variable length elements

            /* Complex Types */
            STRUCT,
            METHOD,

            // New additions should be added at the end to prevent the file hash from changing.

            INAVLID
        };

        // get_type returns the type's fundamental type as an integer constant.
        inline Type get_type() const;

        // has_fixed_size returns true if the DistributedType has a fixed size in bytes.
        inline bool has_fixed_size() const;
        // get_size returns the size of the DistributedType in bytes or 0 if it is variable.
        inline sizetag_t get_size() const;

        // as_numeric returns this as a NumericType if it is numeric, or NULL otherwise.
        virtual NumericType* as_numeric();
        virtual const NumericType* as_numeric() const;

        // as_array returns this as an ArrayType if it is an array, or NULL otherwise.
        virtual ArrayType* as_array();
        virtual const ArrayType* as_array() const;

        // as_struct returns this as a StructType if it is a struct, or NULL otherwise.
        virtual StructType* as_struct();
        virtual const StructType* as_struct() const;

        // as_method returns this as a MethodType if it is a method, or NULL otherwise.
        virtual MethodType* as_method();
        virtual const MethodType* as_method() const;

        // generate_hash accumulates the properties of this file into the hash.
        virtual void generate_hash(HashGenerator& hashgen) const;

    protected:
        Type m_type;
        sizetag_t m_size;
        bool m_has_fixed_size;
};


} // close namespace dclass
