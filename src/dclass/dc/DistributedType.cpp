// Filename: DistributedType.ipp
#include "util/HashGenerator.h"

#include "DistributedType.h"
namespace dclass   // open namespace dclass
{

DistributedType* DistributedType::invalid = new DistributedType();

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

// as_struct returns this as a Struct if it is a struct, or NULL otherwise.
Struct* DistributedType::as_struct()
{
    return (Struct*)NULL;
}
const Struct* DistributedType::as_struct() const
{
    return (const Struct*)NULL;
}

// as_method returns this as a Method if it is a method, or NULL otherwise.
Method* DistributedType::as_method()
{
    return (Method*)NULL;
}
const Method* DistributedType::as_method() const
{
    return (const Method*)NULL;
}

// generate_hash accumulates the properties of this field into the hash.
void DistributedType::generate_hash(HashGenerator& hashgen) const
{
    hashgen.add_int(m_type);
    if(has_alias()) {
        hashgen.add_string(m_alias);
    }
}


} // close namespace dclass
