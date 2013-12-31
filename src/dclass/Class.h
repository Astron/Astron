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
#include "Struct.h"
namespace dclass   // open namespace
{


// A Class defines a particular DistributedClass as read from an input .dc file.
//     Classes are advanced structs which provide inheritence as well as a constructor.
//     Unlike basic structs, classes cannot have anonymous fields.
class Class : public Struct
{
	public:
		Class(File *dc_file, const std::string &name);
		~Class();

		// as_class returns the same Declaration pointer converted to a class
		//     pointer, if this is in fact a class; otherwise, returns NULL.
		virtual Class* as_class();
		virtual const Class* as_class() const;

		// get_num_parents returns the number of base classes this class inherits from.
		inline size_t get_num_parents() const;
		// get_parent returns the nth parent class this class inherits from.
		inline Class* get_parent(unsigned int n) const;

		// has_constructor returns true if this class has a constructor method,
		//     or false if it just uses the default constructor.
		inline bool has_constructor() const;
		// get_constructor returns the constructor method for this class if it is defined,
		//     or NULL if the class uses the default constructor.
		inline Field* get_constructor() const;

		// get_num_inherited_fields returns the total declared and inherited fields in this class.
		inline size_t get_num_inherited_fields() const;
		// get_inherited_field returns the <n>th field from all declared and inherited fields.
		inline Field* get_inherited_field(unsigned int n) const;

		void rebuild_inherited_fields();

		void add_parent(Class *parent);

	private:
		Field* m_constructor;
		std::vector<Class*> m_parents;
		std::vector<Field*> m_inherited_fields;
};


} // close namespace dclass
#include "Class.ipp"
