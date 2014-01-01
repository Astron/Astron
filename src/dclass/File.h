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
#include "KeywordList.h"
#include <string>        // std::string
#include <vector>        // std::vector
#include <unordered_map> // std::unordered_map
namespace dclass   // open namespace
{


// Forward declarations
class Struct;
class Class;
class Field;
class HashGenerator;
class Typedef;
class Keyword;
class Declaration;


// A File represents the complete list of Distributed Class descriptions as read from a .dc file.
class File
{
	public:
		File();
		~File();

		// write writes a parseable description of all the known distributed classes to the output.
		//     Returns true if the description is successfully written, false otherwise.
		bool write(std::string filename, bool brief) const;
		bool write(std::ostream &out, bool brief) const;

		// get_num_classes returns the number of classes read from the .dc file(s).
		int get_num_classes() const;
		// get_class returns the nth class read from the .dc file(s).
		Struct* get_class(int n) const;
		// get_class_by_name returns the requested class or NULL if there is no such class.
		Struct* get_class_by_name(const std::string &name) const;
		// get_field_by_id returns a pointer to the Field that has index number <id>.
		//     Returns NULL if no field exists in the file with that id.
		Field* get_field_by_id(int id) const;

		// get_num_import_modules returns the number of import lines read from the .dc file(s).
		int get_num_import_modules() const;
		// get_import_module returns the module named by the nth import line read from the .dc file(s).
		std::string get_import_module(int n) const;
		// get_num_import_symbols returns the number of symbols explicitly imported by the nth import.
		//     If this is 0, the line imports the entire module.
		int get_num_import_symbols(int n) const;
		// get_import_module returns the module named by the nth import line read from the .dc file(s).
		std::string get_import_symbol(int n, int i) const;

		// get_num_typedefs returns the number of typedefs read from the .dc file(s).
		int get_num_typedefs() const;
		// get_typedef returns the nth typedef read from the .dc file(s).
		Typedef *get_typedef(int n) const;
		// get_typedef_by_name returns the typedef that has the indicated name,
		//     or NULL if there is no such typedef name.
		Typedef *get_typedef_by_name(const std::string &name) const;

		// get_num_keywords returns the number of keywords read from the .dc file(s).
		int get_num_keywords() const;
		// get_keyword returns the nth keyword read from the .dc file(s).
		const Keyword *get_keyword(int n) const;

		// get_keyword_by_name returns the keyword that has the indicated name,
		//     or NULL if there is no such keyword name.
		const Keyword *get_keyword_by_name(const std::string &name) const;

		// get_hash returns a 32-bit hash of the file.
		uint32_t get_hash() const;

		// generate_hash accumulates the properties of this file into the hash.
		void generate_hash(HashGenerator &hashgen) const;

		// add_class adds the newly-allocated distributed class definition to the file.
		//     Returns true if the class is successfully added, or false if there was a name conflict.
		bool add_class(Class *dclass);

		// add_struct adds the newly-allocated struct definition to the file.
		//     Returns true if the class is successfully added, or false if there was a name conflict.
		bool add_struct(Struct *dstruct);

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

		// add_field gives the field a unique id within the file.
		//     Field ids are sequentially assigned as the field is called.
		//     This is only meant to be called by Class::add_field().
		void add_field(Field *field);

		// update_inheritance updates the field inheritance of all classes inheriting from <dclass>.
		void update_inheritance(Class* dclass);

	private:
		void setup_default_keywords();
		void rebuild_inherited_fields();

		std::vector<Struct*> m_classes; // list of classes associated with the file
		std::unordered_map<std::string, Declaration*> m_things_by_name; // classes in the file by name

		class Import
		{
			public:
				std::string m_module;
				std::vector<std::string> m_symbols;
		};

		std::vector<Import> m_imports; // list of python imports in the file
		std::vector<Typedef*> m_typedefs; // list of typedefs in the file

		std::unordered_map<std::string, Typedef*> m_typedefs_by_name; // typedefs in the file by name

		KeywordList m_keywords; // list of keywords read from the file
		KeywordList m_default_keywords; // list of keywords in the file, provided for legacy reasons

		std::vector<Declaration*> m_declarations; // composite list of classes and switches in the file
		std::vector<Declaration*> m_things_to_delete; // list of objects to delete in destructor

		std::vector<Field*> m_fields_by_id; // fields in the file by unique integer id
};


} // close namespace dclass
