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
#include <string>        // std::string
#include <vector>        // std::vector
#include <unordered_map> // std::unordered_map
namespace dclass   // open namespace
{


// Forward declarations
class DistributedType;
class Class;
class StructType;
class Field;
class HashGenerator;

struct Import
{
	std::string module;
	std::vector<std::string> symbols;

	inline Import(const std::string& module_name);
};

// A File represents the complete list of Distributed Class descriptions as read from a .dc file.
class File
{

	public:
		File(); // constructor
		~File(); // destructor

		// get_num_classes returns the number of classes in the file
		inline size_t get_num_classes() const;
		// get_class returns the <n>th class read from the .dc file(s).
		inline Class* get_class(unsigned int n);
		inline const Class* get_class(unsigned int n) const;

		// get_num_structs returns the number of structs in the file.
		inline size_t get_num_structs() const;
		// get_struct returns the <n>th struct in the file.
		inline StructType* get_struct(unsigned int n);
		inline const StructType* get_struct(unsigned int n) const;

		// get_type_by_id returns the requested type or NULL if there is no such type.
		inline DistributedType* get_type_by_id(unsigned int id);
		inline const DistributedType* get_type_by_id(unsigned int id) const;
		// get_type_by_name returns the requested type or NULL if there is no such type.
		inline DistributedType* get_type_by_name(const std::string &name);
		inline const DistributedType* get_type_by_name(const std::string &name) const;

		// get_field_by_id returns the request field or NULL if there is no such field.
		inline Field* get_field_by_id(unsigned int id);
		inline const Field* get_field_by_id(unsigned int id) const;

		// get_num_imports returns the number of imports in the file.
		inline size_t get_num_imports() const;
		// get_import retuns the <n>th import in the file.
		inline Import* get_import(unsigned int n);
		inline const Import* get_import(unsigned int n) const;

		// has_keyword returns true if a keyword with the name <keyword> is declared in the file.
		inline bool has_keyword(const std::string& keyword) const;
		// get_num_keywords returns the number of keywords declared in the file.
		inline size_t get_num_keywords() const;
		// get_keyword returns the <n>th keyword declared in the file.
		inline const std::string& get_keyword(unsigned int n) const;

		// add_class adds the newly-allocated class to the file.
		//     Returns false if there is a name conflict.
		bool add_class(Class *dclass);

		// add_struct adds the newly-allocated struct definition to the file.
		//     Returns false if there is a name conflict.
		bool add_struct(StructType *dstruct);

		// add_typedef adds the alias <name> to the file for the type <type>.
		//     Returns false if there is a name conflict.
		bool add_typedef(const std::string& name, DistributedType* type);

		// add_import adds a newly-allocated import to the file.
		//     Imports with duplicate modules are combined.
		void add_import(Import* import);

		// add_keyword adds a keyword with the name <keyword> to the list of declared keywords.
		void add_keyword(const std::string &keyword);

		// get_hash returns a 32-bit hash representing the file.
		inline uint32_t get_hash() const;

		// generate_hash accumulates the properties of this file into the hash.
		virtual void generate_hash(HashGenerator& hashgen) const;

	private:
		// add_field gives the field a unique id within the file.
		void add_field(Field *field);
		friend class Class;
		friend class StructType;

		std::vector<StructType*> m_structs;
		std::vector<Class*> m_classes;
		std::vector<Import*> m_imports; // list of python imports in the file
		std::vector<std::string> m_keywords;

		std::vector<Field*> m_fields_by_id;
		std::vector<DistributedType*> m_types_by_id;
		std::unordered_map<std::string, DistributedType*> m_types_by_name;
};


} // close namespace dclass
#include "File.ipp"
