// Filename: ArrayParameter.h
// Created by: drose (17Jun04)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "Parameter.h"
#include "NumericRange.h"
namespace dclass   // open namespace
{


// An ArrayParameter represents an array of some other kind of object, meaning
//     this parameter type accepts an arbitrary (or possibly fixed) number of
//     nested fields, all of which are of the same type.
class EXPCL_DIRECT ArrayParameter : public Parameter
{
	public:
		ArrayParameter(Parameter *element_type, const UintRange &size = UintRange());
		ArrayParameter(const ArrayParameter &copy);
		virtual ~ArrayParameter();

	PUBLISHED:
		virtual ArrayParameter *as_array_parameter();
		virtual const ArrayParameter *as_array_parameter() const;
		virtual Parameter *make_copy() const;
		virtual bool is_valid() const;

		Parameter *get_element_type() const;
		int get_array_size() const;

	public:
		virtual Parameter *append_array_specification(const UintRange &size);

		virtual int calc_num_nested_fields(size_t length_bytes) const;
		virtual PackerInterface *get_nested_field(int n) const;
		virtual bool validate_num_nested_fields(int num_nested_fields) const;

		virtual void output_instance(ostream &out, bool brief, const string &prename,
		                             const string &name, const string &postname) const;
		virtual void generate_hash(HashGenerator &hashgen) const;
		virtual void pack_string(PackData &pack_data, const string &value,
		                         bool &pack_error, bool &range_error) const;
		virtual bool pack_default_value(PackData &pack_data, bool &pack_error) const;
		virtual void unpack_string(const char *data, size_t length, size_t &p,
		                           string &value, bool &pack_error, bool &range_error) const;

	protected:
		virtual bool do_check_match(const PackerInterface *other) const;
		virtual bool do_check_match_simple_parameter(const SimpleParameter *other) const;
		virtual bool do_check_match_class_parameter(const ClassParameter *other) const;
		virtual bool do_check_match_array_parameter(const ArrayParameter *other) const;

	private:
		Parameter *m_element_type;
		int m_array_size;
		UintRange m_array_size_range;
};


} // close namespace dclass
