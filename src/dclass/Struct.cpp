#include "Struct.h"
#include "File.h"
#include "HashGenerator.h"
namespace dclass   // open namespace dclass
{

// constructor
Struct::Struct(File* file, const std::string& name) : m_file(file), m_name(name), m_id(0)
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