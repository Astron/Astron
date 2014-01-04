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

// An DistributedType represents any part of a .dc file which can have 
class DistributedType
{
    protected:
        inline DistributedType();
        inline DistributedType(const DistributedType& copy);

    public:
        enum Type
        {
            /* Numeric Types */
            DT_int8,    DT_int16,   DT_int32,  DT_int64,
            DT_uint8,   DT_uint16,  DT_uint32, DT_uint64,
            DT_float32, DT_float64,
            DT_char,        // equivalent to uint8, except that it should be printed as a string

            /* Array Types */
            DT_string,      // a human-printable string with fixed length
            DT_varstring,   // a human-printable string with variable length
            DT_blob,        // any binary data stored as a string, fixed length
            DT_varblob,     // any binary data stored as a varstring, variable length
            DT_array,       // any array with fixed byte-length (fixed array-size and element-length)
            DT_vararray,    // any array with variable array-size or variable length elements

            /* Complex Types */
            DT_struct,
            DT_method,

            // New additions should be added at the end to prevent the file hash from changing.

            DT_invalid
        };

        // get_type returns the type's fundamental type as an integer constant.
        inline Type get_type() const;

        // has_fixed_size returns true if the DistributedType has a fixed size in bytes.
        inline bool has_fixed_size() const;
        // get_size returns the size of the DistributedType in bytes or 0 if it is variable.
        inline sizetag_t get_size() const;

        // as_number returns this as a NumericType if it is numeric, or NULL otherwise.
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

    protected:
        Type m_type;
        sizetag_t m_size;
        bool m_has_fixed_size;
};


} // close namespace dclass
