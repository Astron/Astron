// Filename: DataType.h

#pragma once
#include <iostream>
namespace dclass   // open namespace dclass
{


#ifdef DCLASS_32BIT_LENGTH_TAG
    typedef uint32_t sizetag_t;
#else
    typedef uint16_t sizetag_t;
#endif

// A DataType defines the numeric type of any distributed element.
//     This type specifies the layout of the element in memory.
enum DataType
{
    DT_int8,
    DT_int16,
    DT_int32,
    DT_int64,

    DT_uint8,
    DT_uint16,
    DT_uint32,
    DT_uint64,

    DT_float32,
    DT_float64,

    DT_char,        // equivalent to uint8, except that it should be printed as a string

    DT_string,      // a human-printable string with fixed length
    DT_varstring,   // a human-printable string with variable length
    DT_blob,        // any binary data stored as a string, fixed length
    DT_varblob,     // any binary data stored as a varstring, variable length
    DT_array,       // any array with fixed byte-length (fixed array-size and element-length)
    DT_vararray,    // any array with variable array-size or variable length elements

    DT_method,
    DT_struct,

    // New additions should be added at the end to prevent the file hash from changing.

    DT_invalid
};


std::ostream &operator << (std::ostream &out, DataType type);


} // close namespace dclass
