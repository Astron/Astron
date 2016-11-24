// Filename: Parameter.cpp
#include "util/HashGenerator.h"
#include "value/default.h"
#include "dc/Struct.h"
#include "dc/Method.h"

#include "Parameter.h"
namespace dclass   // open namespace dclass
{


// constructor
Parameter::Parameter(DistributedType* type, const std::string& name) :
    m_name(name), m_type(type), m_method(nullptr), m_has_default_value(false)
{
    bool implicit_value;
    m_default_value = create_default_value(type, implicit_value);
    m_has_default_value = !implicit_value;

    if(m_type == nullptr) {
        m_type = DistributedType::invalid;
    }
}

// set_name sets the name of this parameter.  Returns false if a parameter with
//     the same name already exists in the containing method.
bool Parameter::set_name(const std::string& name)
{
    // Check to make sure no other fields in our struct have this name
    if(m_method != nullptr && m_method->get_parameter_by_name(name) != nullptr) {
        return false;
    }

    m_name = name;
    return true;
}

// set_type sets the distributed type of the parameter and clear's the default value.
bool Parameter::set_type(DistributedType* type)
{
    if(type == nullptr) {
        return false;
    }

    // Parameters can't have method types for now
    if(type->get_type() == T_METHOD) {
        return false;
    }

    // Parameters can't have class types for now
    if(type->get_type() == T_STRUCT && type->as_struct()->as_class()) {
        return false;
    }

    m_type = type;
    m_has_default_value = false;
    m_default_value = create_default_value(type);

    return true;
}

// set_default_value defines a default value for this parameter.
//     Returns false if the value is invalid for the parameter's type.
bool Parameter::set_default_value(const std::string& default_value)
{
    // TODO: Validate default value
    m_has_default_value = true;
    m_default_value = default_value;
    return true;
}

// set_method sets a pointer to the method containing the parameter.
void Parameter::set_method(Method* method)
{
    m_method = method;
}

// generate_hash accumulates the properties of this type into the hash.
void Parameter::generate_hash(HashGenerator& hashgen) const
{
    m_type->generate_hash(hashgen);
}


} // close namespace dclass
