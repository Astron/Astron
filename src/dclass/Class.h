// Filename: Class.h
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
#include "Field.h"
#include "Declaration.h"
#include "Element.h"
#include <vector> // for std::vector
#include <unordered_map> // for std::unordered_map
namespace dclass   // open namespace
{


// Foward declaration
class HashGenerator;
class File;
//class Parameter;

// A Class (dclass::Class) defines a particular DistributedClass as read from an input .dc file.
class Class : public Declaration, public Element
{
	friend class Field;

	public:
		Class(File *dc_file, const std::string &name,
		      bool is_struct, bool bogus_class);
		~Class();

		// as_class returns the same Declaration pointer converted to a class
		//     pointer, if this is in fact a class; otherwise, returns NULL.
		virtual Class *as_class();
		virtual const Class *as_class() const;

		// get_file returns the File object that contains the class.
		inline File *get_file() const;

		// get_name returns the name of this class.
		inline const std::string &get_name() const;
		// get_number returns a unique index number associated with this class.
		//     This is defined implicitly when a .dc file is read.
		inline int get_number() const;

		// get_num_parents returns the number of base classes this class inherits from.
		size_t get_num_parents() const;
		// get_parent returns the nth parent class this class inherits from.
		Class *get_parent(unsigned int n) const;

		// has_constructor returns true if this class has a constructor method,
		//     or false if it just uses the default constructor.
		bool has_constructor() const;
		// get_constructor returns the constructor method for this class if it
		//     is defined, or NULL if the class uses the default constructor.
		Field *get_constructor() const;

		// get_num_fields returns the number of fields defined directly, ignoring inherited fields.
		size_t get_num_fields() const;
		// get_field returns the nth field in the class.  This is not the field with index n;
		//     this is the nth field defined in the class directly, ignoring inheritance.
		Field *get_field(unsigned int n) const;

		// get_field_by_name returns a pointer to the declared or inherited Field with name 'name';
		//     Returns NULL if there is no such field defined.
		Field *get_field_by_name(const std::string &name) const;
		// get_field_by_index returns a pointer to the declared or
		//     inherited Field with unique id 'index';
		//     Returns NULL if there is no such field defined.
		Field *get_field_by_index(int index);
		Field *get_field_by_index(int index) const;

		// get_num_inherited_fields returns the total declared and inherited fields in this class.
		size_t get_num_inherited_fields();
		size_t get_num_inherited_fields() const;

		// get_inherited_field returns the nth field from all
		//     declared and inherited fields in the class.
		Field *get_inherited_field(int n);
		Field *get_inherited_field(int n) const;

		// is_struct returns true if the class has been identified with the "struct"
		//     keyword in the dc file, false if it was declared with "dclass".
		inline bool is_struct() const;
		// is_bogus_class returns true if the class has been flagged as a bogus class.
		//     This is used by the parser as a placeholder for missing classes.
		inline bool is_bogus_class() const;
		// inherits_from_bogus_class returns true if this class, or any class in the inheritance
		//     heirarchy for this class, is a forward reference to an as-yet-undefined class.
		bool inherits_from_bogus_class() const;

		virtual void output(std::ostream &out) const;

		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;
		void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                     const std::string &name, const std::string &postname) const;
		void generate_hash(HashGenerator &hashgen) const;
		void clear_inherited_fields();
		void rebuild_inherited_fields();

		bool add_field(Field *field);
		void add_parent(Class *parent);
		void set_number(int number);

	private:
		void shadow_inherited_field(const std::string &name);

		File *m_file;

		std::string m_name;
		bool m_is_struct;
		bool m_bogus_class;
		int m_number;

		Field* m_constructor;
		std::vector<Class*> m_parents;
		std::vector<Field*> m_fields, m_inherited_fields;
		std::unordered_map<std::string, Field*> m_fields_by_name;
		std::unordered_map<int, Field*> m_fields_by_index;
};


} // close namespace dclass

#include "Class.ipp"
