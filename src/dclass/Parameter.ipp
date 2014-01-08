// Filename: Parameter.ipp
namespace dclass   // open namespace
{


// get_name returns the parameter's name.  An unnamed parameter returns the empty string.
inline const std::string& Parameter::get_name() const
{
	return m_name;
}

// get_type returns the DistributedType of the Parameter.
inline DistributedType* Parameter::get_type()
{
	return m_type;
}
inline const DistributedType* Parameter::get_type() const
{
	return m_type;
}

// get_method returns the Method that contains the Parameter.
inline Method* Parameter::get_method()
{
	return m_method;
}
inline const Method* Parameter::get_method() const
{
	return m_method;
}

// has_default_value returns true if a default value was defined for this parameter.
inline bool Parameter::has_default_value() const
{
	return m_has_default_value;
}

// get_default_value returns the default value for this parameter.
//     If a default value hasn't been set, returns an implicit default.
inline const std::string& Parameter::get_default_value() const
{
	return m_default_value;
}


} // close namespace dclass
