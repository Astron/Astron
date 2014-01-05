// Filename: Field.h
// Created by: drose (11 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "KeywordList.h"
namespace dclass   // open namespace
{

// Foward declarations
class DistributedType;
class StructType;
class HashGenerator;

// A Field is a member of a class or struct.
class Field : public KeywordList
{
	public:
		Field(StructType* strct, const std::string &name = "");

		// get_id returns a unique index number associated with this field.
		inline unsigned int get_id() const;
		// get_name returns the field's name.  An unnamed field returns the empty string.
		inline const std::string& get_name() const;
		// get_type returns the DistributedType of the field.
		inline DistributedType* get_type();
		inline const DistributedType* get_type() const;
		// get_struct returns the StructType that contains this field.
		inline StructType* get_struct();
		inline const StructType* get_struct() const;

		// has_default_value returns true if a default value was defined for this field.
		inline bool has_default_value() const;
		// get_default_value returns the default value for this field.
		//     If a default value hasn't been set, returns an implicit default.
		inline const std::string& get_default_value() const;

		// set_name sets the name of this field.  Returns false if a field with
		//     the same name already exists in the containing struct.
		bool set_name(const std::string& name);

		// set_type sets the distributed type of the field and clear's the default value.
		void set_type(DistributedType* type);

		// set_default_value defines a default value for this field.
		//     Returns false if the value is invalid for the field's type.
		bool set_default_value(const std::string& default_value);

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator& hashgen) const;

	private:
		// set_id sets the unique index number associated with the field.
		void set_id(unsigned int id);
		friend bool File::add_field(Field* field);

		// set_struct sets a pointer to the struct containing the field.
		void set_struct(StructType* strct);
		friend bool StructType::add_field(Field* field);

		unsigned int m_id;
		std::string m_name;
		DistributedType* m_type;
		StructType* m_struct;

		bool m_has_default_value; // is true if an explicity default has been set
		std::string m_default_value; // the binary data of the default value encoded in a string
};


} // close namespace dclass
#include "Field.ipp"
