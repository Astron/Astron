// Filename: Method.ipp
namespace dclass   // open namespace
{


// get_num_parameters returns the number of parameters/arguments of the method.
inline size_t Method::get_num_parameters() const
{
	return m_parameters.size();
}
// get_element returns the <n>th parameter of the method.
inline Parameter* Method::get_parameter(unsigned int n)
{
	return m_parameters.at(n);
}
inline const Parameter* Method::get_parameter(unsigned int n) const\
{
	return m_parameters.at(n);
}

// get_parameter_by_name returns the parameter with <name>, or nullptr if no such param exists.
inline Parameter* Method::get_parameter_by_name(const std::string& name)
{
	auto it = m_parameters_by_name.find(name);
	if(it == m_parameters_by_name.end())
	{
		return nullptr;
	}
	return it->second;
}
inline const Parameter* Method::get_parameter_by_name(const std::string& name) const
{
	auto it = m_parameters_by_name.find(name);
	if(it == m_parameters_by_name.end())
	{
		return nullptr;
	}
	return it->second;
}


} // close namespace dclass
