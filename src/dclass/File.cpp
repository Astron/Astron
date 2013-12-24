// Filename: File.cpp
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "File.h"
#include "Class.h"
#include "parser/ParserDefs.h"
#include "parser/LexerDefs.h"
#include "Typedef.h"
#include "Keyword.h"
#include "HashGenerator.h"
#include <assert.h>
#include <fstream>
namespace dclass   // open namespace
{


// constructor
File::File() : m_all_objects_valid(true), m_inherited_fields_stale(false)
{
	setup_default_keywords();
}

//destructor
File::~File()
{
	clear();
}

// clear removes all of the classes defined within the
//     File and prepares it for reading a new file.
void File::clear()
{
	for(auto it = m_declarations.begin(); it != m_declarations.end(); ++it)
	{
		delete(*it);
	}
	for(auto it = m_things_to_delete.begin(); it != m_things_to_delete.end(); ++it)
	{
		delete(*it);
	}

	m_classes.clear();
	m_imports.clear();
	m_things_by_name.clear();
	m_typedefs.clear();
	m_typedefs_by_name.clear();
	m_keywords.clear_keywords();
	m_declarations.clear();
	m_things_to_delete.clear();
	setup_default_keywords();

	m_all_objects_valid = true;
	m_inherited_fields_stale = false;
}

// read opens and reads the indicated .dc file by name.  The distributed classes defined
//     in the file will be appended to the set of distributed classes already recorded, if any.
//     Returns true if the file is successfully read, false if there was an error.
bool File::read(std::string filename)
{
	std::ifstream in;
	in.open(filename.c_str());

	if(!in)
	{
		std::cerr << "Cannot open " << filename << " for reading.\n";
		return false;
	}

	return read(in, filename);
}

// read parses the already-opened input stream for distributed class descriptions.
//     The filename parameter is optional and is only used when reporting errors.
//     The distributed classes defined in the file will be appended to the set of
//     distributed classes already recorded, if any.
//     Returns true if the file is successfully read, false if there was an error.
bool File::read(std::istream &in, const std::string &filename)
{
	dc_init_parser(in, filename, *this);
	dcyyparse();
	dc_cleanup_parser();

	return (dc_error_count() == 0);
}

// write opens the indicated filename for output and writes a parseable
//     description of all the known distributed classes to the file.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(std::string filename, bool brief) const
{
	std::ofstream out;

	out.open(filename.c_str());

	if(!out)
	{
		std::cerr << "Can't open " << filename << " for output.\n";
		return false;
	}
	return write(out, brief);
}

// write writes a parseable description of all the known distributed classes to the stream.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(std::ostream &out, bool brief) const
{
	if(!m_imports.empty())
	{
		for(auto imp_it = m_imports.begin(); imp_it != m_imports.end(); ++imp_it)
		{
			const Import &import = (*imp_it);
			if(import.m_symbols.empty())
			{
				out << "import " << import.m_module << "\n";
			}
			else
			{
				out << "from " << import.m_module << " import ";
				auto sym_it = import.m_symbols.begin();
				out << *sym_it;
				++sym_it;
				while(sym_it != import.m_symbols.end())
				{
					out << ", " << *sym_it;
					++sym_it;
				}
				out << "\n";
			}
		}
		out << "\n";
	}


	for(auto it = m_declarations.begin(); it != m_declarations.end(); ++it)
	{
		(*it)->write(out, brief, 0);
		out << "\n";
	}

	return !out.fail();
}

// get_num_classes returns the number of classes read from the .dc file(s).
int File::get_num_classes() const
{
	return m_classes.size();
}

// get_class returns the nth class read from the .dc file(s).
Class* File::get_class(int n) const
{
	assert(n >= 0 && n < (int)m_classes.size());
	return m_classes[n];
}

// get_class_by_name returns the class that has the indicated name,
//     or NULL if there is no such class.
Class* File::get_class_by_name(const std::string &name) const
{
	auto class_it = m_things_by_name.find(name);
	if(class_it != m_things_by_name.end())
	{
		return class_it->second->as_class();
	}

	return (Class*)NULL;
}

// get_field_by_index returns a pointer to the one Field that has the indicated
//     index number, of all the Fields across all classes in the file.
//     This method is only valid if dc-multiple-inheritance is set true in the
//     Config.prc file.  Without this setting, different Fields may share the
//     same index number, so this global lookup is not possible.
Field* File::get_field_by_index(int index_number) const
{
	assert(dc_multiple_inheritance);

	if(index_number >= 0 && index_number < (int)m_fields_by_index.size())
	{
		return m_fields_by_index[index_number];
	}

	return NULL;
}

// get_num_import_modules returns the number of import lines read from the .dc file(s).
int File::get_num_import_modules() const
{
	return m_imports.size();
}

// get_import_module returns the module named by the nth import line read from the .dc file(s).
std::string File::get_import_module(int n) const
{
	assert(n >= 0 && n < (int)m_imports.size());
	return m_imports[n].m_module;
}

// get_num_import_symbols returns the number of symbols explicitly imported by
//     the nth import line.  If this is 0, the line is "import modulename";
//     if it is more than 0, the line is "from modulename import symbol, symbol ... ".
int File::get_num_import_symbols(int n) const
{
	assert(n >= 0 && n < (int)m_imports.size());
	return m_imports[n].m_symbols.size();
}

// get_import_symbol returns the ith symbol named by the nth import line read from the .dc file(s).
std::string File::get_import_symbol(int n, int i) const
{
	assert(n >= 0 && n < (int)m_imports.size());
	assert(i >= 0 && i < (int)m_imports[n].m_symbols.size());
	return m_imports[n].m_symbols[i];
}

// get_num_typedefs returns the number of typedefs read from the .dc file(s).
int File::get_num_typedefs() const
{
	return m_typedefs.size();
}

// get_typedef returns the nth typedef read from the .dc file(s).
Typedef *File::get_typedef(int n) const
{
	assert(n >= 0 && n < (int)m_typedefs.size());
	return m_typedefs[n];
}

// get_typedef_by_name returns the typedef that has the indicated name,
//     or NULL if there is no such typedef name.
Typedef *File::get_typedef_by_name(const std::string &name) const
{
	auto typ_it = m_typedefs_by_name.find(name);
	if(typ_it != m_typedefs_by_name.end())
	{
		return typ_it->second;
	}

	return NULL;
}

// get_num_keywords returns the number of keywords read from the .dc file(s).
int File::get_num_keywords() const
{
	return m_keywords.get_num_keywords();
}

// get_keyword returns the nth keyword read from the .dc file(s).
const Keyword *File::get_keyword(int n) const
{
	return m_keywords.get_keyword(n);
}

// get_keyword_by_name returns the keyword that has the indicated name,
//     or NULL if there is no such keyword name.
const Keyword *File::get_keyword_by_name(const std::string &name) const
{
	const Keyword *keyword = m_keywords.get_keyword_by_name(name);
	if(keyword == (const Keyword*)NULL)
	{
		keyword = m_default_keywords.get_keyword_by_name(name);
		if(keyword != (const Keyword*)NULL)
		{
			// One of the historical default keywords was used, but wasn't
			// defined.  Define it implicitly right now.
			((File*)this)->m_keywords.add_keyword(keyword);
		}
	}

	return keyword;
}

// get_hash returns a 32-bit hash index associated with this file.
//     This number is guaranteed to be consistent if the contents of the file
//     have not changed, and it is very likely to be different if the
//     contents of the file do change.
uint32_t File::get_hash() const
{
	HashGenerator hashgen;
	generate_hash(hashgen);
	return hashgen.get_hash();
}

// generate_hash accumulates the properties of this file into the hash.
void File::generate_hash(HashGenerator &hashgen) const
{
	// Legacy requires adding this number right now
	hashgen.add_int(1);
	hashgen.add_int(m_classes.size());
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		(*it)->generate_hash(hashgen);
	}
}

// add_class adds the newly-allocated distributed class definition to the file.
//     The File becomes the owner of the pointer and will delete it when it destructs.
//     Returns true if the class is successfully added, or false if there was a name conflict.
bool File::add_class(Class *dclass)
{
	if(!dclass->get_name().empty())
	{
		bool inserted = m_things_by_name.insert(
			std::map<std::string, Declaration*>::value_type(dclass->get_name(), dclass)).second;

		if(!inserted)
		{
			return false;
		}
	}

	if(!dclass->is_struct())
	{
		dclass->set_number(get_num_classes());
	}
	m_classes.push_back(dclass);

	if(dclass->is_bogus_class())
	{
		m_all_objects_valid = false;
	}

	if(!dclass->is_bogus_class())
	{
		m_declarations.push_back(dclass);
	}
	else
	{
		m_things_to_delete.push_back(dclass);
	}

	return true;
}

// add_import_module adds a new name to the list of names of Python modules that
//     are to be imported by the client or AI to define the code that is associated
//     with the class interfaces named within the .dc file.
void File::add_import_module(const std::string &import_module)
{
	Import import;
	import.m_module = import_module;
	m_imports.push_back(import);
}

// add_import_symbol adds a new name to the list of symbols that are to be
//     explicitly imported from the most-recently added module, e.g.
//     "from module_name import symbol".  If the list of symbols is empty,
//     the syntax is taken to  be "import module_name".
void File::add_import_symbol(const std::string &import_symbol)
{
	assert(!m_imports.empty());
	m_imports.back().m_symbols.push_back(import_symbol);
}

// add_typedef adds the newly-allocated distributed typedef definition to the file.
//     The File becomes the owner of the pointer and will delete it when it destructs.
//     Returns true if the typedef is successfully added, or  false if there was a name conflict.
bool File::add_typedef(Typedef *dtypedef)
{
	bool inserted = m_typedefs_by_name.insert(
		std::map<std::string, Typedef*>::value_type(dtypedef->get_name(), dtypedef)).second;

	if(!inserted)
	{
		return false;
	}

	dtypedef->set_number(get_num_typedefs());
	m_typedefs.push_back(dtypedef);

	if(dtypedef->is_bogus_typedef())
	{
		m_all_objects_valid = false;
	}

	if(!dtypedef->is_bogus_typedef() && !dtypedef->is_implicit_typedef())
	{
		m_declarations.push_back(dtypedef);
	}
	else
	{
		m_things_to_delete.push_back(dtypedef);
	}

	return true;
}

// add_keyword adds the indicated keyword string to the list of keywords known to the File.
//     These keywords may then be added to Fields.
//     It is not an error to add a particular keyword more than once.
bool File::add_keyword(const std::string &name)
{
	Keyword *keyword = new Keyword(name);
	bool added = m_keywords.add_keyword(keyword);

	if(added)
	{
		m_declarations.push_back(keyword);
	}
	else
	{
		delete keyword;
	}

	return added;
}

// add_thing_to_delete adds the indicated declaration to the list of declarations
//     that are not reported with the file, but will be deleted when the File object destructs.
//     That is, transfers ownership of the indicated pointer to the File.
void File::add_thing_to_delete(Declaration *decl)
{
	m_things_to_delete.push_back(decl);
}

// set_new_index_number sets the next sequential available index number on the indicated field.
//     This is only meant to be called by Class::add_field(), while the dc file is being parsed.
void File::set_new_index_number(Field *field)
{
	field->set_number((int)m_fields_by_index.size());
	m_fields_by_index.push_back(field);
}

// setup_default_keywords adds an entry for each of the default keywords
//     that are defined for every File for legacy reasons.
void File::setup_default_keywords()
{
	struct KeywordDef
	{
		const char *name;
		int flag;
	};
	static KeywordDef default_keywords[] =
	{
		{ "required", 0x0001 },
		{ "broadcast", 0x0002 },
		{ "ownrecv", 0x0004 },
		{ "ram", 0x0008 },
		{ "db", 0x0010 },
		{ "clsend", 0x0020 },
		{ "clrecv", 0x0040 },
		{ "ownsend", 0x0080 },
		{ "airecv", 0x0100 },
		{ NULL, 0 }
	};

	m_default_keywords.clear_keywords();
	for(int i = 0; default_keywords[i].name != NULL; ++i)
	{
		Keyword *keyword =
		    new Keyword(default_keywords[i].name,
		                  default_keywords[i].flag);

		m_default_keywords.add_keyword(keyword);
		m_things_to_delete.push_back(keyword);
	}
}

// rebuild_inherited_fields reconstructs the inherited fields table of all classes.
void File::rebuild_inherited_fields()
{
	m_inherited_fields_stale = false;

	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		(*it)->clear_inherited_fields();
	}
	for(auto it = m_classes.begin(); it != m_classes.end(); ++it)
	{
		(*it)->rebuild_inherited_fields();
	}
}


} // close namespace dclass