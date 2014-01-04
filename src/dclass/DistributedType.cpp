// Filename: DistributedType.ipp
#include "DistributedType.h"
namespace dclass   // open namespace dclass
{


// as_number returns this as a NumericType if it is numeric, or NULL otherwise.
virtual NumericType* as_numeric()
{
    return (NumericType*)NULL;
}
virtual const NumericType* as_numeric() const
{
    return (const NumericType*)NULL;
}

// as_array returns this as an ArrayType if it is an array, or NULL otherwise.
virtual ArrayType* as_array()
{
    return (ArrayType*)NULL;
}
virtual const ArrayType* as_array() const
{
    return (const ArrayType*)NULL;
}

// as_struct returns this as a StructType if it is a struct, or NULL otherwise.
virtual StructType* as_struct()
{
    return (StructType*)NULL;
}
virtual const StructType* as_struct() const
{
    return (const StructType*)NULL;
}

// as_method returns this as a MethodType if it is a method, or NULL otherwise.
virtual MethodType* as_method()
{
    return (MethodType*)NULL;
}
virtual const MethodType* as_method() const
{
    return (const MethodType*)NULL;
}


} // close namespace dclass
