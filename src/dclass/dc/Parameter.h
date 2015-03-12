// Filename: Parameter.h
#pragma once
#include <string> // std::string
namespace dclass   // open namespace dclass
{


// Foward declarations
class DistributedType;
class Method;
class HashGenerator;

// A Parameter is a single argument/parameter of a Method.
class Parameter
{
  public:
    Parameter(DistributedType* type, const std::string& name = "");

    // get_name returns the parameter's name.  An unnamed parameter returns the empty string.
    inline const std::string& get_name() const;
    // get_type returns the DistributedType of the Parameter.
    inline DistributedType* get_type();
    inline const DistributedType* get_type() const;
    // get_method returns the Method that contains the Parameter.
    inline Method* get_method();
    inline const Method* get_method() const;

    // has_default_value returns true if a default value was defined for this parameter.
    inline bool has_default_value() const;
    // get_default_value returns the default value for this parameter.
    //     If a default value hasn't been set, returns an implicit default.
    inline const std::string& get_default_value() const;

    // set_name sets the name of this parameter.  Returns false if a parameter with
    //     the same name already exists in the containing method.
    bool set_name(const std::string& name);

    // set_type sets the distributed type of the parameter and clear's the default value.
    //     Returns false if a parameter cannot represent <type>.
    bool set_type(DistributedType* type);

    // set_default_value defines a default value for this parameter.
    //     Returns false if the value is invalid for the parameter's type.
    bool set_default_value(const std::string& default_value);

    // generate_hash accumulates the properties of this type into the hash.
    void generate_hash(HashGenerator& hashgen) const;

  private:
    // set_method sets a pointer to the method containing the parameter.
    void set_method(Method* method);
    friend class Method;

    std::string m_name;
    std::string m_type_alias;
    DistributedType* m_type; // cannot be a Method object
    Method* m_method;

    bool m_has_default_value; // is true if an explicity default has been set
    std::string m_default_value; // the binary data of the default value encoded in a string
};


} // close namespace dclass
#include "Parameter.ipp"
