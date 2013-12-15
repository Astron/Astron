// Filename: Switch.h
// Created by: drose (23 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "Declaration.h"
#include "PackerInterface.h"
namespace dclass   // open namespace dclass
{


// Foward declaration
class Parameter;
class HashGenerator;
class Field;

// A Switch represents a switch statement, which can appear inside a class body and
//     represents two or more alternative unpacking schemes based on the first field read.
class EXPCL_DIRECT Switch : public Declaration
{
	public:
		Switch(const std::string &name, Field *key_parameter);
		virtual ~Switch();

	PUBLISHED:
		virtual Switch *as_switch();
		virtual const Switch *as_switch() const;

		const string &get_name() const;
		Field *get_key_parameter() const;

		int get_num_cases() const;
		int get_case_by_value(const std::string &case_value) const;
		PackerInterface *get_case(int n) const;
		PackerInterface *get_default_case() const;

		string get_value(int case_index) const;
		int get_num_fields(int case_index) const;
		Field *get_field(int case_index, int n) const;
		Field *get_field_by_name(int case_index, const std::string &name) const;

	public:
		bool is_field_valid() const;
		int add_case(const std::string &value);
		void add_invalid_case();
		bool add_default();
		bool add_field(Field *field);
		void add_break();

		const PackerInterface *apply_switch(const char *value_data, size_t length) const;

		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;
		void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                     const std::string &name, const std::string &postname) const;
		void write_instance(std::ostream &out, bool brief, int indent_level,
		                    const std::string &prename, const std::string &name,
		                    const std::string &postname) const;
		virtual void generate_hash(HashGenerator &hashgen) const;
		virtual bool pack_default_value(PackData &pack_data, bool &pack_error) const;

		bool do_check_match_switch(const Switch *other) const;

	public:
		class SwitchFields : public PackerInterface
		{
			public:
				SwitchFields(const std::string &name);
				~SwitchFields();
				virtual PackerInterface *get_nested_field(int n) const;

				bool add_field(Field *field);
				bool do_check_match_switch_case(const SwitchFields *other) const;

				void output(ostream &out, bool brief) const;
				void write(ostream &out, bool brief, int indent_level) const;

			protected:
				virtual bool do_check_match(const PackerInterface *other) const;

			public:
				std::vector<Field*> m_fields;
				std::map<std::string, Field*> m_fields_by_name;
				bool m_has_default_value;
		};

		class SwitchCase
		{
			public:
				SwitchCase(const std::string &value, SwitchFields *fields);
				~SwitchCase();

				bool do_check_match_switch_case(const SwitchCase *other) const;

			public:
				std::string m_value;
				SwitchFields *m_fields;
		};

	private:
		SwitchFields *start_new_case();

	private:
		std::string m_name;
		Field *m_key_parameter;

		std::vector<SwitchCase*> m_cases;
		SwitchFields *m_default_case;

		// All SwitchFields created and used by the Switch object are also
		// stored here; this is the vector that "owns" the pointers.
		std::vector<SwitchFields*> m_case_fields;

		// All nested Field objects that have been added to one or more of
		// the above SwitchFields are also recorded here; this is the vector
		// that "owns" these pointers.
		std::vector<Field*> m_nested_fields;

		// These are the SwitchFields that are currently being filled up
		// during this stage of the parser.  There might be more than one at
		// a time, if we have multiple cases being introduced in the middle
		// of a series of fields (without a break statement intervening).
		CaseFields m_current_fields;
		bool m_fields_added;

		// This map indexes into the _cases vector, above.
		std::map<std::string, int> m_cases_by_value;
};


} // close namespace dclass
