// Filename: MethodType.h
#pragma once
#include "DistributedType.h"
namespace dclass   // open namespace
{


// A Method is a field for a distributed Class that typically represents a remote procedure call.
class MethodType : public DistributedType
{
	public:
		MethodType();
		virtual ~MethodType();

        // as_method returns this as a MethodType if it is a method, or NULL otherwise.
        virtual MethodType* as_method();
        virtual const MethodType* as_method() const;

		// get_num_parameters returns the number of parameters/arguments of the method.
		inline size_t get_num_parameters() const;
		// get_element returns the <n>th parameter of the method.
		inline Parameter* get_parameter(unsigned int n);
		inline const Parameter* get_parameter(unsigned int n) const;

		// add_parameter adds a new parameter to the method.
		void add_parameter(Parameter *param);

		// generate_hash accumulates the properties of this field into the hash
		virtual void generate_hash(HashGenerator &hashgen) const;

	private:
		std::vector<Parameter*> m_parameters; // the "arguments" or parameters of the method
};


} // close namespace dclass
#include "MethodType.ipp"
