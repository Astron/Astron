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
#include "PackerInterface.h"
#include "KeywordList.h"
namespace dclass   // open namespace
{


// Forward declarations
class Packer;
class AtomicField;
class MolecularField;
class Parameter;
class Switch;
class Class;
class HashGenerator;

// A Field is a single field of a Distributed Class, either atomic or molecular.
class EXPCL_DIRECT Field : public PackerInterface, public KeywordList
{
	public:
		Field();
		Field(const string &name, Class *dclass);
		virtual ~Field();

	PUBLISHED:
		// get_number returns a unique index number associated with this field.
		//     This is defined implicitly when the .dc file(s) are read.
		inline int get_number() const;

		// get_class returns the Class pointer for the class that contains this field.
		inline Class *get_class() const;


		virtual Field *as_field();
		virtual const Field *as_field() const;
		virtual AtomicField *as_atomic_field();
		virtual const AtomicField *as_atomic_field() const;
		virtual MolecularField *as_molecular_field();
		virtual const MolecularField *as_molecular_field() const;
		virtual Parameter *as_parameter();
		virtual const Parameter *as_parameter() const;



		string format_data(const string &packed_data, bool show_field_names = true);
		string parse_string(const string &formatted_string);

		bool validate_ranges(const string &packed_data) const;


		// has_default_value returns true if a default value has been explicitly
		//     established for this field, false otherwise.
		inline bool has_default_value() const;

		// get_default_value returns the default value for this field.
		//     If a default value hasn't been set, returns an implicit default.
		inline const string &get_default_value() const;

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
		inline void output(ostream &out) const;
		inline void write(ostream &out, int indent_level) const;
	public:
		// output and write output a string representation of this instance to <out>.
		virtual void output(ostream &out, bool brief) const = 0;
		virtual void write(ostream &out, bool brief, int indent_level) const = 0;

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;
		virtual bool pack_default_value(PackData &pack_data, bool &pack_error) const;
		virtual void set_name(const string &name);

		inline void set_number(int number);
		inline void set_class(Class *dclass);
		inline void set_default_value(const string &default_value);

	protected:
		void refresh_default_value();

	protected:
		Class *m_class;
		int m_number;
		bool m_default_value_stale;
		bool m_has_default_value;
		bool m_bogus_field;

	private:
		string m_default_value;

};

inline ostream &operator << (ostream &out, const Field &field)
{
	field.output(out);
	return out;
}



} // close namespace dclass

#include "Field.ipp"
