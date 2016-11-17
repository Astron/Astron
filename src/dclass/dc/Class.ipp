// Filename: Class.ipp
namespace dclass   // open namespace
{


// get_num_parents returns the number of superclasses this class inherits from.
inline size_t Class::get_num_parents() const
{
	return m_parents.size();
}
// get_parent returns the <n>th parent-/super-class this class inherits from.
inline Class* Class::get_parent(unsigned int n)
{
	return m_parents.at(n);
}
inline const Class* Class::get_parent(unsigned int n) const
{
	return m_parents.at(n);
}

// get_num_children returns the number of subclasses that inherit from this class.
inline size_t Class::get_num_children() const
{
	return m_children.size();
}
// get_child returns the <n>th child-/sub-class that inherits this class.
inline Class* Class::get_child(unsigned int n)
{
	return m_children.at(n);
}
inline const Class* Class::get_child(unsigned int n) const
{
	return m_children.at(n);
}

// has_constructor returns true if this class has a constructor method,
//     or false if it just uses the default constructor.
inline bool Class::has_constructor() const
{
	return (m_constructor != nullptr);
}
// get_constructor returns the constructor method for this class if it is defined,
//     or nullptr if the class uses the default constructor.
inline Field* Class::get_constructor()
{
	return m_constructor;
}
inline const Field* Class::get_constructor() const
{
	return m_constructor;
}

// get_num_base_fields returns the number of fields declared directly in this class.
inline size_t Class::get_num_base_fields() const
{
	return m_base_fields.size();
}
// get_base_field returns the <n>th field from the class excluding any inherited fields.
inline Field* Class::get_base_field(unsigned int n)
{
	return m_base_fields.at(n);
}
inline const Field* Class::get_base_field(unsigned int n) const
{
	return m_base_fields.at(n);
}


} // close namespace dclass
