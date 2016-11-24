// Filename: Method.h
#pragma once
#include <stddef.h>      // size_t
#include <vector>        // std::vector
#include <unordered_map> // std::unordered_map

#include "DistributedType.h"
namespace dclass   // open namespace
{

// Forward declarations
class Parameter;

// A Method is a field for a distributed Class that typically represents a remote procedure call.
class Method final : public DistributedType
{
  public:
    Method();
    ~Method();

    // as_method returns this as a Method if it is a method, or nullptr otherwise.
    Method* as_method();
    const Method* as_method() const;

    // get_num_parameters returns the number of parameters/arguments of the method.
    size_t get_num_parameters() const;
    // get_par returns the <n>th parameter of the method.
    Parameter* get_parameter(unsigned int n);
    const Parameter* get_parameter(unsigned int n) const;
    // get_parameter_by_name returns the requested parameter or nullptr if there is no such param.
    Parameter* get_parameter_by_name(const std::string& name);
    const Parameter* get_parameter_by_name(const std::string& name) const;

    // add_parameter adds a new parameter to the method.
    //     Returns false if the parameter could not be added to the method.
    bool add_parameter(Parameter *param);

    // generate_hash accumulates the properties of this field into the hash
    void generate_hash(HashGenerator &hashgen) const;

  private:
    std::vector<Parameter*> m_parameters; // the "arguments" or parameters of the method
    std::unordered_map<std::string, Parameter*> m_parameters_by_name;
};


} // close namespace dclass
#include "Method.ipp"
