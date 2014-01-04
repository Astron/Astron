// Filename: DistributedType.ipp
#include "DistributedType.h"
#include "HashGenerator.h"
namespace dclass   // open namespace dclass
{


// as_number returns this as a NumericType if it is numeric, or NULL otherwise.
NumericType* as_numeric()
{
	return (NumericType*)NULL;
}
const NumericType* as_numeric() const
{
	return (const NumericType*)NULL;
}

// as_array returns this as an ArrayType if it is an array, or NULL otherwise.
ArrayType* as_array()
{
	return (ArrayType*)NULL;
}
const ArrayType* as_array() const
{
	return (const ArrayType*)NULL;
}

// as_struct returns this as a StructType if it is a struct, or NULL otherwise.
StructType* as_struct()
{
	return (StructType*)NULL;
}
const StructType* as_struct() const
{
	return (const StructType*)NULL;
}

// as_method returns this as a MethodType if it is a method, or NULL otherwise.
MethodType* as_method()
{
	return (MethodType*)NULL;
}
const MethodType* as_method() const
{
	return (const MethodType*)NULL;
}

// generate_hash accumulates the properties of this field into the hash.
void DistributedType::generate_hash(HashGenerator& hashgen) const
{
	hashgen.add_int(m_type);
}


} // close namespace dclass
