#include "Struct.h"
#include "File.h"
#include "HashGenerator.h"
#include "indent.h" // temporary
using namespace std;
namespace dclass   // open namespace dclass
{

// constructor
Struct::Struct(File* file, const string& name) : m_file(file), m_name(name), m_id(0)
{
}

// parameter constructer
Struct::Struct(const Struct* base) : m_file(base->m_file), m_name(base->m_name), m_id(base->m_id)
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

// as_struct returns the same pointer converted to a struct pointer
//     if this is in fact a struct; otherwise, returns NULL.
Struct* Struct::as_struct()
{
	return this;
}
const Struct* Struct::as_struct() const
{
	return this;
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
	if(m_has_fixed_size && !field->as_molecular_field())
	{
		if(field->has_fixed_size())
		{
			m_bytesize += field->get_size();
		}
		else
		{
			m_has_fixed_size = false;
			m_bytesize = 0;
		}
	}
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


/* TODO: Move and update appropriately */
// output formats a string representation of the class in .dc file syntax
//     as "dclass IDENTIFIER" or "struct IDENTIFIER" with structs having optional IDENTIFIER,
//     and outputs the formatted string to the stream.
void Struct::output(ostream &out) const
{
	if(this->as_class())
	{
		out << "dclass";
	}
	else
	{
		out << "struct";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}
}

// output formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::output(ostream &out, bool brief) const
{
	output_instance(out, brief, "", "", "");
}

// write formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::write(ostream &out, bool brief, int indent_level) const
{
	indent(out, indent_level);
	if(this->as_class())
	{
		out << "dclass";
	}
	else
	{
		out << "struct";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}

	// TODO: Move and update class write
	/*
	if(!m_parents.empty())
	{
		auto it = m_parents.begin();
		out << " : " << (*it)->m_name;
		++it;
		while(it != m_parents.end())
		{
			out << ", " << (*it)->m_name;
			++it;
		}
	}
	*/

	out << " {";
	if(!brief && m_id >= 0)
	{
		out << "  // index " << m_id;
	}
	out << "\n";

	// TODO: Move and update class writing
	/*
	if(m_constructor != (Field*)NULL)
	{
		m_constructor->write(out, brief, indent_level + 2);
	}
	*/

	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->write(out, brief, indent_level + 2);

		
		//if (true || (*fi)->has_default_value()) {
		//  indent(out, indent_level + 2) << "// = ";
		//  Packer packer;
		//  packer.set_unpack_data((*fi)->get_default_value());
		//  packer.begin_unpack(*fi);
		//  packer.unpack_and_format(out, false);
		//  if (!packer.end_unpack()) {
		//    out << "<error>";
		//  }
		//  out << "\n";
		//}
	}

	indent(out, indent_level) << "};\n";
}

// output_instance formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::output_instance(ostream &out, bool brief, const string &prename,
                            const string &name, const string &postname) const
{
	if(this->as_class())
	{
		out << "dclass";
	}
	else
	{
		out << "struct";
	}
	if(!m_name.empty())
	{
		out << " " << m_name;
	}

	/*
	if(!m_parents.empty())
	{
		auto it = m_parents.begin();
		out << " : " << (*it)->m_name;
		++it;
		while(it != m_parents.end())
		{
			out << ", " << (*it)->m_name;
			++it;
		}
	}
	*/

	out << " {";

	/*
	if(m_constructor != (Field*)NULL)
	{
		m_constructor->output(out, brief);
		out << "; ";
	}
	*/

	for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		(*it)->output(out, brief);
		out << "; ";
	}

	out << "}";
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
}


} // close namespace dclass