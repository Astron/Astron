// Filename: File.ipp
namespace dclass   // open namespace
{


// get_num_classes returns the number of classes in the file
inline size_t File::get_num_classes() const
{
	return m_classes.size();
}
// get_class returns the <n>th class read from the .dc file(s).
inline Class* File::get_class(unsigned int n)
{
	return m_classes.at(n);
}
inline const Class* File::get_class(unsigned int n) const
{
	return m_classes.at(n);
}

// get_num_structs returns the number of structs in the file.
inline size_t File::get_num_structs() const
{
	return m_structs.size();
}
// get_struct returns the <n>th struct in the file.
inline StructType* File::get_struct(unsigned int n)
{
	return m_structs.at(n);
}
inline const StructType* File::get_struct(unsigned int n) const
{
	return m_structs.at(n);
}

// get_type_by_id returns the requested type or NULL if there is no such type.
inline DistributedType* File::get_type_by_id(unsigned int id)
{
	if(id < m_types_by_id.size())
	{
		return m_types_by_id[id];
	}

	return (DistributedType*)NULL;
}
inline const DistributedType* File::get_type_by_id(unsigned int id) const
{
	if(id < m_types_by_id.size())
	{
		return m_types_by_id[id];
	}

	return (const DistributedType*)NULL;
}
// get_type_by_name returns the requested type or NULL if there is no such type.
inline DistributedType* File::get_type_by_name(const std::string &name)
{
	auto type_ref = m_types_by_name.find(name);
	if(type_ref != m_types_by_name.end())
	{
		return type_ref->second;
	}

	return (DistributedType*)NULL;
}
inline const DistributedType* File::get_type_by_name(const std::string &name) const
{
	auto type_ref = m_types_by_name.find(name);
	if(type_ref != m_types_by_name.end())
	{
		return type_ref->second;
	}

	return (const DistributedType*)NULL;
}

// get_field_by_id returns the request field or NULL if there is no such field.
inline Field* File::get_field_by_id(unsigned int id)
{
	if(id < m_fields_by_id.size())
	{
		return m_fields_by_id[id];
	}

	return (Field*)NULL;
}
inline const Field* File::get_field_by_id(unsigned int id) const
{
	if(id < m_fields_by_id.size())
	{
		return m_fields_by_id[id];
	}

	return (const Field*)NULL;
}

// get_num_imports returns the number of imports in the file.
inline size_t File::get_num_imports() const
{
	return m_imports.size();
}
// get_import retuns the <n>th import in the file.
inline Import* File::get_import(unsigned int n)
{
	return m_imports.at(n);
}
inline const Import* File::get_import(unsigned int n) const
{
	return m_imports.at(n);
}

// has_keyword returns true if a keyword with the name <keyword> is declared in the file.
inline bool File::has_keyword(const std::string& keyword) const
{
	for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
	{
		if(*it == keyword)
		{
			return true;
		}
	}
	return false;
}
// get_num_keywords returns the number of keywords declared in the file.
inline size_t File::get_num_keywords() const
{
	return m_keywords.size();
}
// get_keyword returns the <n>th keyword declared in the file.
inline const std::string& File::get_keyword(unsigned int n) const
{
	return m_keywords.at(n);
}


} // close namespace dclass
