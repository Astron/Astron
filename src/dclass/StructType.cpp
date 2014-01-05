#include "StructType.h"
#include "HashGenerator.h"
using namespace std;
namespace dclass   // open namespace dclass
{

// constructor
StructType::StructType(File* file, const string& name) : m_file(file), m_name(name), m_id(0)
{
	m_type = STRUCT;
}

// destructor
StructType::~StructType()
{
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		delete(*it);
	}
}

// as_struct returns this as a StructType if it is a StructType, or NULL otherwise.
StructType* StructType::as_struct()
{
	return this;
}
const StructType* StructType::as_struct() const
{
	return this;
}

// as_class returns this StructType as a Class if it is a Class, or NULL otherwise.
Class* StructType::as_class()
{
	return (Class*)NULL;
}
const Class* StructType::as_class() const
{
	return (const Class*)NULL;
}

// add_field adds a new Field to the struct.
bool StructType::add_field(Field* field)
{
	// Structs can't share a field
	if(field->get_struct() != NULL && field->get_struct() != this)
	{
		return false;
	}

	// Structs can't have moleculars
	if(field->as_molecular())
	{
		return false;
	}

	// Structs can't have methods
	if(field->get_type()->as_method())
	{
		return false;
	}

	// Struct fields are accessible by name
	if(!field->get_name().empty())
	{
		// Structs can't have Constructors
		if(field->get_name() == m_name)
		{
			return false;
		}

		// Try to add the field
		bool inserted = m_fields_by_name.insert(
			unordered_map<string, Field*>::value_type(field->get_name(), field)).second;

		if(!inserted)
		{
			// But the field had a name conflict
			return false;
		}
	}

	// Struct fields are accessible by id.
	m_file->add_field(field);
	m_fields_by_id.insert(unordered_map<int, Field*>::value_type(field->get_id(), field)).second;

	m_fields.push_back(field);
	if(has_fixed_size() || m_fields.size() == 1)
	{
		if(field->get_type()->has_fixed_size())
		{
			m_size += field->get_type()->get_size();
		}
		else
		{
			m_size = 0;
		}
	}
	return true;
}

// generate_hash accumulates the properties of this class into the hash.
void StructType::generate_hash(HashGenerator& hashgen) const
{
	DistributedType::generate_hash(hashgen);
	hashgen.add_string(m_name);
	hashgen.add_int(m_fields.size());
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}


} // close namespace dclass