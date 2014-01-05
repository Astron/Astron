// Filename: MethodType.cpp
#include "MethodType.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// constructor
MethodType::MethodType()
{
	m_type = METHOD;
}

// destructor
MethodType::~MethodType()
{
	for(auto it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		delete(*it);
	}
	m_parameters.clear();
}

// as_method returns this as a MethodType if it is a method, or NULL otherwise.
MethodType* MethodType::as_method()
{
	return this;
}
const MethodType* MethodType::as_method() const
{
	return this;
}

// get_num_parameters returns the number of parameters/arguments of the method.
inline size_t MethodType::get_num_parameters() const
{
	return m_parameters.size();
}
// get_element returns the <n>th parameter of the method.
inline Parameter* MethodType::get_parameter(unsigned int n)
{
	return m_parameters.at(n);
}
inline const Parameter* MethodType::get_parameter(unsigned int n) const\
{
	return m_parameters.at(n);
}

// add_parameter adds a new parameter to the method.
void MethodType::add_parameter(Parameter *param)
{
	m_parameters.push_back(param);
	if(has_fixed_size() || m_parameters.size() == 1)
	{
		if(param->get_type()->has_fixed_size())
		{
			m_size += param->get_type()->get_size();
		}
		else
		{
			m_size = 0;
		}
	}
}

// generate_hash accumulates the properties of this method into the hash
void MethodType::generate_hash(HashGenerator& hashgen) const
{
	DistributedType::generate_hash(hashgen);
	hashgen.add_int(m_.size());
	for(auto it = m_parameters.begin(); it != m_parameters.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}


} // close namespace dclass
