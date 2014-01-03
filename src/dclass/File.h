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
#include <unordered_set> // std::unordered_set
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

struct Import
{
	std::string module;
	std::vector<std::string> symbols;

	inline Import(const std::string& module_name);
};

// A File represents the complete list of Distributed Class descriptions as read from a .dc file.
class File : Hashable
{

	public:
		File(); // constructor
		~File(); // destructor

		// get_num_classes returns the number of classes in the file
		size_t get_num_classes() const;
		// get_class returns the <n>th class read from the .dc file(s).
		Class* get_class(unsigned int n);
		const Class* get_class(unsigned int n) const;

		// get_num_structs returns the number of structs in the file.
		size_t get_num_structs() const;
		// get_struct returns the <n>th struct in the file.
		Struct* get_struct(unsigned int n);
		const Struct* get_struct(unsigned int n) const;

		// get_type_by_id returns the requested type or NULL if there is no such type.
		DistributedType* get_type_by_id(unsigned int id);
		const DistributedType* get_type_by_id(unsigned int id) const;
		// get_type_by_name returns the requested type or NULL if there is no such type.
		DistributedType* get_type_by_name(const std::string &name);
		const DistributedType* get_type_by_name(const std::string &name) const;

		// get_field_by_id returns the request field or NULL if there is no such field.
		Field* get_field_by_id(unsigned int id);
		const Field* get_field_by_id(unsigned int id) const;

		// get_num_imports returns the number of imports in the file.
		size_t get_num_imports() const;
		// get_import retuns the <n>th import in the file.
		Import* get_import(unsigned int n);
		const Import* get_import(unsigned int n) const;

		// has_keyword returns true if a keyword with the name <keyword> is declared in the file.
		bool has_keyword(const std::string& keyword) const;
		// get_num_keywords returns the number of keywords declared in the file.
		int get_num_keywords() const;
		// get_keyword returns the <n>th keyword declared in the file.
		const std::string& *get_keyword(int n) const;

		// add_class adds the newly-allocated class to the file.
		//     Returns false if there is a name conflict.
		bool add_class(Class *dclass);

		// add_struct adds the newly-allocated struct definition to the file.
		//     Returns false if there is a name conflict.
		bool add_struct(Struct *dstruct);

		// add_typedef adds the alias <name> to the file for the type <type>.
		//     Returns false if there is a name conflict.
		bool add_typedef(const std::string& name, DistributedType* type);

		// add_keyword adds a keyword with the name <keyword> to the list of declared keywords.
		void add_keyword(const std::string &name);

		// add_import adds a newly-allocated import to the file.
		//     Imports with duplicate modules are combined.
		void add_import_module(Import* import);

		// update_inheritance updates the field inheritance of all classes inheriting from <dclass>.
		void update_inheritance(Class* dclass);

	private:
		// add_field gives the field a unique id within the file.
		void add_field(Field *field);
		friend bool Class::add_field(Field* field);
		friend bool Struct::add_field(Field* field);

		std::vector<Struct*> m_structs;
		std::vector<Class*> m_classes;
		std::unordered_map<std::string, DistributedType*> m_type_by_name;
		std::unordered_map<unsigned int, DistributedType*> m_type_by_id;

		std::vector<Field*> m_fields_by_id;

		std::vector<Import> m_imports; // list of python imports in the file
		std::unordered_set<std::string> m_keywords;

};


} // close namespace dclass
