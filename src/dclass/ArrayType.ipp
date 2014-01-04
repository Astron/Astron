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

// has_range returns true if the numeric is constrained by a range.
inline bool ArrayType::has_range() const
{
	return !m_array_range.is_empty();
}

// get_array_size returns the fixed number of elements in this array,
//     or 0 if the array may contain a variable number of elements.
inline size_t ArrayType::get_array_size() const
{
	return m_array_size;
}


} // close namespace dclass
