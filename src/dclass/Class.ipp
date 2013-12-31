// Filename: Class.ipp
#include <assert.h>
namespace dclass   // open namespace
{


// get_num_parents returns the number of base classes this class inherits from.
inline size_t Class::get_num_parents() const
{
	return m_parents.size();
}
// get_parent returns the nth parent class this class inherits from.
inline Class* Class::get_parent(unsigned int n) const
{
	assert(n < m_parents.size());
	return m_parents[n];
}

// has_constructor returns true if this class has a constructor method,
//     or false if it just uses the default constructor.
inline bool Class::has_constructor() const
{
	return (m_constructor != (Field*)NULL);
}
// get_constructor returns the constructor method for this class if it is defined,
//     or NULL if the class uses the default constructor.
inline Field* Class::get_constructor() const
{
	return m_constructor;
}

// get_num_base_fields returns the number of fields declared directly in this class.
inline size_t Class::get_num_base_fields() const
{
	return m_base_fields.size();
}

// get_base_field returns the <n>th field from the class excluding any inherited fields.
inline Field* Class::get_base_field(unsigned int n) const
{
	assert(n < m_base_fields.size());
	return m_base_fields[n];
}


} // close namespace dclass
