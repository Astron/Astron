#include "Struct.h"
#include "File.h"
#include "HashGenerator.h"
using namespace std;
namespace dclass   // open namespace dclass
{

// constructor
Struct::Struct(File* file, const string& name) : m_file(file), m_name(name), m_id(0)
{
}

// destructor
Struct::~Struct()
{
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		delete(*it);
	}
}

bool Struct::add_field(Field* field)
{	
	// Try to add the field
	bool inserted = m_fields_by_name.insert(
		unordered_map<string, Field*>::value_type(field->get_name(), field)).second;

	if(!inserted)
	{
		// But the field had a name conflict
		return false;
	}

	// If we're using a File
	if(m_file != (File*)NULL)
	{
		// Then all of our fields are indexed
		m_file->add_field(field);
		m_fields_by_id.insert(unordered_map<int, Field*>::value_type(field->get_id(), field)).second;
	}

	m_fields.push_back(field);
	m_bytesize += field->get_size();
	return true;
}


// generate_hash accumulates the properties of this class into the hash.
void Struct::generate_hash(HashGenerator &hashgen) const
{
	hashgen.add_string(m_name);
	if(!this->as_class())
	{
		hashgen.add_int(1);
	}

	/* TODO: Move to appropriate place
	 *

	hashgen.add_int(m_parents.size());
	for(auto it = m_parents.begin(); it != m_parents.end(); ++it)
	{
		hashgen.add_int((*it)->get_number());
	}

	if(m_constructor != (Field*)NULL)
	{
		m_constructor->generate_hash(hashgen);
	}
	*/

	hashgen.add_int(m_fields.size());
	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}


} // close namespace dclass