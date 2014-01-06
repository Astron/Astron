// Filename: Parameter.h
#pragma once
#include <string> // std::string
namespace dclass   // open namespace dclass
{


// Foward declarations
class DistributedType;
class MethodType;
class HashGenerator;

// A Parameter is a single argument/parameter of a Method.
class Parameter
{
	public:
		Parameter(const std::string& name = "");

		// get_name returns the parameter's name.  An unnamed parameter returns the empty string.
		inline const std::string& get_name() const;
		// get_type returns the DistributedType of the Parameter.
		inline DistributedType* get_type();
		inline const DistributedType* get_type() const;
		// get_method returns the MethodType that contains the Parameter.
		inline MethodType* get_method();
		inline const MethodType* get_method() const;

		// has_default_value returns true if a default value was defined for this parameter.
		inline bool has_default_value() const;
		// get_default_value returns the default value for this parameter.
		//     If a default value hasn't been set, returns an implicit default.
		inline const std::string& get_default_value() const;

		// has_type_alias returns true if this parameter was defined with an alias of a type.
		inline bool has_type_alias() const;
		// get_type_alias returns the type name used to define the parameter, or the empty string.
		inline const std::string& get_type_alias() const;

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
		void set_method(MethodType* method);
		friend class MethodType;

		std::string m_name;
		std::string m_type_alias;
		DistributedType* m_type; // cannot be a MethodType object
		MethodType* m_method;

		bool m_has_default_value; // is true if an explicity default has been set
		std::string m_default_value; // the binary data of the default value encoded in a string
};


} // close namespace dclass
#include "Parameter.ipp"
