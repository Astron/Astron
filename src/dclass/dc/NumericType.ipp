// Filename: NumericType.ipp
namespace dclass   // open namespace
{


// get_divisor returns the divisor of the numeric, with a default value of one.
inline unsigned int NumericType::get_divisor() const
{
	return m_divisor;
}

// has_modulus returns true if the numeric is constrained by a modulus.
inline bool NumericType::has_modulus() const
{
	return m_orig_modulus != 0.0;
}

// get_modulus returns a double representation of the modulus value.
inline double NumericType::get_modulus() const
{
	return m_orig_modulus;
}

// has_range returns true if the numeric is constrained by a range.
inline bool NumericType::has_range() const
{
	return !m_orig_range.is_empty();
}
// get_range returns the NumericRange that constrains the type's values.
inline NumericRange NumericType::get_range() const
{
	return m_orig_range;
}


} // close namespace dclass
