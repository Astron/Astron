// Filename: ArrayType.ipp
namespace dclass   // open namespace
{


// get_element_type returns the type of the individual elements of this array.
inline DistributedType* ArrayType::get_element_type()
{
	return m_element_type;
}
inline const DistributedType* ArrayType::get_element_type() const
{
	return m_element_type;
}

// get_array_size returns the fixed number of elements in this array,
//     or 0 if the array may contain a variable number of elements.
inline size_t ArrayType::get_array_size() const
{
	return m_array_size;
}

// has_range returns true if there is a constraint on the range of valid array sizes.
//     This is always true for fixed-size arrays.
inline bool ArrayType::has_range() const
{
	return !m_array_range.is_empty();
}

// get_range returns the range of sizes that the array may have.
inline NumericRange ArrayType::get_range() const
{
	return m_array_range;
}


} // close namespace dclass
