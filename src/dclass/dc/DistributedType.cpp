// Filename: DistributedType.ipp
#include "util/HashGenerator.h"

#include "DistributedType.h"
namespace dclass   // open namespace dclass
{

DistributedType* DistributedType::invalid = new DistributedType();

DistributedType::~DistributedType()
{
}

// as_number returns this as a NumericType if it is numeric, or nullptr otherwise.
NumericType* DistributedType::as_numeric()
{
    return nullptr;
}
const NumericType* DistributedType::as_numeric() const
{
    return nullptr;
}

// as_array returns this as an ArrayType if it is an array, or nullptr otherwise.
ArrayType* DistributedType::as_array()
{
    return nullptr;
}
const ArrayType* DistributedType::as_array() const
{
    return nullptr;
}

// as_struct returns this as a Struct if it is a struct, or nullptr otherwise.
Struct* DistributedType::as_struct()
{
    return nullptr;
}
const Struct* DistributedType::as_struct() const
{
    return nullptr;
}

// as_method returns this as a Method if it is a method, or nullptr otherwise.
Method* DistributedType::as_method()
{
    return nullptr;
}
const Method* DistributedType::as_method() const
{
    return nullptr;
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
