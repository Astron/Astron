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
#include "dcbase.h"
#include "Element.h"
#include "KeywordList.h"
namespace dclass   // open namespace
{


// Forward declarations
class AtomicField;
class MolecularField;
class Parameter;
class Class;
class HashGenerator;

// A Field is a single field of a Distributed Class, either atomic or molecular.
class Field : public Element, public KeywordList
{
	public:
		Field();
		Field(const std::string &name, Class *dclass);
		virtual ~Field();

		// get_number returns a unique index number associated with this field.
		//     This is defined implicitly when the .dc file(s) are read.
		inline int get_number() const;

		// get_class returns the Class pointer for the class that contains this field.
		inline Class *get_class() const;


		// as_field returns the same pointer converted to a field pointer,
		//     if this is in fact a field; otherwise, returns NULL.
		virtual Field *as_field();
		virtual const Field *as_field() const;

		// as_atomic_field returns the same field pointer converted to an atomic field
		//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
		virtual AtomicField *as_atomic_field();
		virtual const AtomicField *as_atomic_field() const;

		// as_molecular_field returns the same field pointer converted to a molecular field
		//     pointer, if this is in fact a molecular field; otherwise, returns NULL.
		virtual MolecularField *as_molecular_field();
		virtual const MolecularField *as_molecular_field() const;

		// as_parameter returns the same field pointer converted to a parameter
		//     pointer, if this is in fact a parameter; otherwise, returns NULL.
		virtual Parameter *as_parameter();
		virtual const Parameter *as_parameter() const;

		// validate_ranges verifies that all of the packed values in the field data are
		//     within the specified ranges and that there are no extra bytes on the end
		//     of the record.  Returns true if all fields are valid, false otherwise.
		bool validate_ranges(const std::string &packed_data) const;

		// has_default_value returns true if a default value has been explicitly
		//     established for this field, false otherwise.
		inline bool has_default_value() const;

		// get_default_value returns the default value for this field.
		//     If a default value hasn't been set, returns an implicit default.
		inline const std::string &get_default_value() const;

		// is_bogus_field returns true if the field has been flagged as a bogus field.
		//     This can occur during parsing, but should not occur in a normal valid dc file.
		inline bool is_bogus_field() const;

		// is_required returns true if the "required" flag is set for this field, false otherwise.
		inline bool is_required() const;
		// is_broadcast returns true if the "broadcast" flag is set for this field, false otherwise.
		inline bool is_broadcast() const;
		// is_ram returns true if the "ram" flag is set for this field, false otherwise.
		inline bool is_ram() const;
		// is_db returns true if the "db" flag is set for this field, false otherwise.
		inline bool is_db() const;
		// is_clsend returns true if the "clsend" flag is set for this field, false otherwise.
		inline bool is_clsend() const;
		// is_clrecv returns true if the "clrecv" flag is set for this field, false otherwise.
		inline bool is_clrecv() const;
		// is_ownsend returns true if the "ownsend" flag is set for this field, false otherwise.
		inline bool is_ownsend() const;
		// is_ownrecv returns true if the "ownrecv" flag is set for this field, false otherwise.
		inline bool is_ownrecv() const;
		// is_airecv returns true if the "airecv" flag is set for this field, false otherwise.
		inline bool is_airecv() const;

		// output and write output a string representation of this instance to <out>.
		inline void output(std::ostream &out) const;
		inline void write(std::ostream &out, int indent_level) const;

		// output and write output a string representation of this instance to <out>.
		virtual void output(std::ostream &out, bool brief) const = 0;
		virtual void write(std::ostream &out, bool brief, int indent_level) const = 0;

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

		// set_name sets the name of this field.
		virtual void set_name(const std::string &name);


		// set_number assigns the unique number to this field.  This is normally
		//     called only by the Class interface as the field is added.
		inline void set_number(int number);
		// set_class assigns the class pointer to this field.  This is normally
		//     called only by the Class interface as the field is added.
		inline void set_class(Class *dclass);

		// set_default_value establishes a default value for this field.
		inline void set_default_value(const std::string &default_value);

		inline const std::string & get_name() const
		{
			return m_name;
		}

	protected:
		// refresh_default_value recomputes the default value of the field by repacking it.
		void refresh_default_value();

	protected:
		Class *m_class; // the class that this field belongs to
		std::string m_name;
		int m_number; // the unique index of the field in the .dc file
		bool m_default_value_stale; // is true if the default value hasn't been computed
		bool m_has_default_value; // is true if an explicity default has been set
		bool m_bogus_field; // is true if the field is incomplete (only encountered during parsing)

	private:
		std::string m_default_value; // the binary data of the default value encoded in a string

};

inline std::ostream &operator << (std::ostream &out, const Field &field)
{
	field.output(out);
	return out;
}



} // close namespace dclass

#include "Field.ipp"
