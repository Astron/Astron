// Filename: File.h
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "KeywordList.h"
#include <vector>
#include <map>
namespace dclass   // open namespace
{


// Forward declarations
class Class;
class Field;
class HashGenerator;
class Typedef;
class Keyword;
class Declaration;


// A File represents the complete list of Distributed Class descriptions as read from a .dc file.
class EXPCL_DIRECT File
{
	PUBLISHED:
		File();
		~File();

		void clear();

		// read opens and reads the indicated .dc file by name.
		//     Returns true if the file was successfully read, or false if an error occurs.
		bool read(Filename filename);
		// read opens and parses the istream as a .dc file; the filename is used in debug output.
		//     Returns true if the file was successfully read, or false if an error occurs.
		bool read(std::istream &in, const string &filename = string());

		// write writes a parseable description of all the known distributed classes to the output.
		//     Returns true if the description is successfully written, false otherwise.
		bool write(Filename filename, bool brief) const;
		bool write(ostream &out, bool brief) const;

		// get_num_classes returns the number of classes read from the .dc file(s).
		int get_num_classes() const;
		// get_class returns the nth class read from the .dc file(s).
		Class *get_class(int n) const;
		// get_class_by_name returns the requested class or NULL if there is no such class.
		Class *get_class_by_name(const string &name) const;
		// get_field_by_index returns a pointer to the one Field that has the indicated
		//     index number, of all the Fields across all classes in the file.
		Field *get_field_by_index(int index_number) const;

		// all_objects_valid returns true if all of the classes read from the DC
		//     file were defined and valid, or false if any of them were undefined.
		inline bool all_objects_valid() const;

		// get_num_import_modules returns the number of import lines read from the .dc file(s).
		int get_num_import_modules() const;
		// get_import_module returns the module named by the nth import line read from the .dc file(s).
		string get_import_module(int n) const;
		// get_num_import_symbols returns the number of symbols explicitly imported by the nth import.
		//     If this is 0, the line imports the entire module.
		int get_num_import_symbols(int n) const;
		// get_import_module returns the module named by the nth import line read from the .dc file(s).
		string get_import_symbol(int n, int i) const;

		// get_num_typedefs returns the number of typedefs read from the .dc file(s).
		int get_num_typedefs() const;
		// get_typedef returns the nth typedef read from the .dc file(s).
		Typedef *get_typedef(int n) const;
		// get_typedef_by_name returns the typedef that has the indicated name,
		//     or NULL if there is no such typedef name.
		Typedef *get_typedef_by_name(const string &name) const;

		// get_num_keywords returns the number of keywords read from the .dc file(s).
		int get_num_keywords() const;
		// get_keyword returns the nth keyword read from the .dc file(s).
		const Keyword *get_keyword(int n) const;

		// get_keyword_by_name returns the keyword that has the indicated name,
		//     or NULL if there is no such keyword name.
		const Keyword *get_keyword_by_name(const std::string &name) const;

		// get_hash returns a 32-bit hash of the file.
		uint32_t get_hash() const;

	public:
		// generate_hash accumulates the properties of this file into the hash.
		void generate_hash(HashGenerator &hashgen) const;

		// add_class adds the newly-allocated distributed class definition to the file.
		//     Returns true if the class is successfully added, or false if there was a name conflict.
		bool add_class(Class *dclass);

		// add_import_module adds a new name to the list of names of Python modules.
		void add_import_module(const std::string &import_module);

		// add_import_symbol adds a new name to the list of symbols that are to be
		//     explicitly imported from the most-recently added module.
		void add_import_symbol(const std::string &import_symbol);

		// add_import_symbol adds a new name to the list of symbols that are to be
		//     explicitly imported from the most-recently added module.
		bool add_typedef(Typedef *dtypedef);

		// add_keyword adds the indicated keyword string to the list of keywords known to the File.
		bool add_keyword(const std::string &name);

		// add_thing_to_delete transdfers ownership of a decl to the file so that it will
		//     be deleted when the file's destructor is called.
		void add_thing_to_delete(Declaration *decl);

		// set_new_index_number sets the next sequential available index number on the indicated field.
		void set_new_index_number(Field *field);

		// check_inherited_fields rebuilds all of the inherited fields tables, if necessary.
		inline void check_inherited_fields();

		// mark_inherited_fields_stale indicates that something has changed in one or more
		//     of the inheritance chains or the set of fields.
		inline void mark_inherited_fields_stale();

	private:
		void setup_default_keywords();
		void rebuild_inherited_fields();

		std::vector<Class*> m_classes; // list of classes associated with the file
		std::map<std::string, Declaration*> m_things_by_name; // classes in the file by name

		class Import
		{
			public:
				string m_module;
				std::vector<string> m_symbols;
		};

		std::vector<Import> m_imports; // list of python imports in the file
		std::vector<Typedef*> m_typedefs; // list of typedefs in the file

		std::map<string, Typedef*> m_typedefs_by_name; // typedefs in the file by name

		KeywordList m_keywords; // list of keywords read from the file
		KeywordList m_default_keywords; // list of keywords in the file, provided for legacy reasons

		std::vector<Declaration*> m_declarations; // composite list of classes and switches in the file
		std::vector<Declaration*> m_things_to_delete; // list of objects to delete in destructor

		std::vector<Field*> m_fields_by_index; // fields in the file by unique integer id

		bool m_all_objects_valid; // true if there are no object has an incomplete defintion.
		bool m_inherited_fields_stale; // true if inherited fields have not be calculated.
};


} // close namespace dclass

#include "File.ipp"
