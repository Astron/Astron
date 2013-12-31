// Filename: Struct.ipp
#include <assert.h>
namespace dclass   // open namespace
{

// get_file returns the File object that contains the struct.
inline File* Struct::get_file() const
{
	return m_file;
}

// get_name returns the name of this struct.
inline const std::string& Struct::get_name() const
{
	return m_name;
}

// get_id returns a unique index number associated with this struct.
inline unsigned int Struct::get_id() const
{
	return m_id;
}

// get_num_fields returns the number of fields defined directly in the struct.
inline size_t Struct::get_num_fields() const
{
	return m_fields.size();
}

// get_field returns the <n>th field defined directly in the struct.
inline Field* Struct::get_field(unsigned int n) const
{
	assert(n < m_fields.size());
	return m_fields[n];
}

// get_field_by_name returns the field with <name>, or NULL if no such field exists.
inline Field* Struct::get_field_by_name(const std::string& name) const
{
	auto it = m_fields_by_name.find(name);
	if(it == m_fields_by_name.end())
	{
		return (Field*)NULL;
	}
	return it->second;
}

// get_field_by_id returns the field with the index <id>, or NULL if no such field exists.
inline Field* Struct::get_field_by_id(unsigned int id) const
{
	auto it = m_fields_by_id.find(id);
	if(it == m_fields_by_id.end())
	{
		return (Field*)NULL;
	}
	return it->second;
}

// set_id sets the index number associated with this struct.
inline void Struct::set_id(unsigned int id)
{
	m_id = id;
}

} // close namespace dclass
