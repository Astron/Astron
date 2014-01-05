// Filename: DistributedType.ipp
#include "DistributedType.h"
#include "HashGenerator.h"
namespace dclass   // open namespace dclass
{

DistributedType::~DistributedType()
{
}

// as_number returns this as a NumericType if it is numeric, or NULL otherwise.
NumericType* DistributedType::as_numeric()
{
	return (NumericType*)NULL;
}
const NumericType* DistributedType::as_numeric() const
{
	return (const NumericType*)NULL;
}

// as_array returns this as an ArrayType if it is an array, or NULL otherwise.
ArrayType* DistributedType::as_array()
{
	return (ArrayType*)NULL;
}
const ArrayType* DistributedType::as_array() const
{
	return (const ArrayType*)NULL;
}

// as_struct returns this as a StructType if it is a struct, or NULL otherwise.
StructType* DistributedType::as_struct()
{
	return (StructType*)NULL;
}
const StructType* DistributedType::as_struct() const
{
	return (const StructType*)NULL;
}

// as_method returns this as a MethodType if it is a method, or NULL otherwise.
MethodType* DistributedType::as_method()
{
	return (MethodType*)NULL;
}
const MethodType* DistributedType::as_method() const
{
	return (const MethodType*)NULL;
}

// generate_hash accumulates the properties of this field into the hash.
void DistributedType::generate_hash(HashGenerator& hashgen) const
{
	hashgen.add_int(m_type);
}


} // close namespace dclass
