// Filename: Method.cpp
#include "util/HashGenerator.h"
#include "dc/Parameter.h"

#include "Method.h"
using namespace std;
namespace dclass   // open namespace
{


// constructor
Method::Method()
{
    m_type = T_METHOD;
}

// destructor
Method::~Method()
{
    for(auto& it : m_parameters) {
        delete it;
    }

    m_parameters.clear();
}

// as_method returns this as a Method if it is a method, or nullptr otherwise.
Method* Method::as_method()
{
    return this;
}
const Method* Method::as_method() const
{
    return this;
}

// add_parameter adds a new parameter to the method.
bool Method::add_parameter(Parameter *param)
{
    // Param should not be nullptr
    if(param == nullptr) {
        return false;
    }

    if(!param->get_name().empty()) {
        // Try to add the parameter
        bool inserted = m_parameters_by_name.insert(
                            unordered_map<string, Parameter*>::value_type(param->get_name(), param)).second;
        if(!inserted) {
            // But the parameter had a name conflict
            return false;
        }
    }

    // Add the parameter to the main list
    param->set_method(this);
    m_parameters.push_back(param);

    // Update our size
    if(has_fixed_size() || m_parameters.size() == 1) {
        if(param->get_type()->has_fixed_size()) {
            m_size += param->get_type()->get_size();
        } else {
            m_size = 0;
        }
    }

    return true;
}

// generate_hash accumulates the properties of this method into the hash
void Method::generate_hash(HashGenerator& hashgen) const
{
    DistributedType::generate_hash(hashgen);
    hashgen.add_int(m_parameters.size());
    for(const auto& it : m_parameters) {
        it->generate_hash(hashgen);
    }
}


} // close namespace dclass
