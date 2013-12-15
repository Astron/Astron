// Filename: SimpleParameter.h
// Created by: drose (15 Jun, 2004)
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
#include "SubatomicType.h"
#include "NumericRange.h"
namespace dclass   // open namespace dclass
{


// A SimpleParameter is the most fundamental kind of parameter type: a single number or string,
//     one of the SubatomicType elements.  It may also optionally have a divisor, which is
//     meaningful only for the numeric type elements (and represents a fixed-point numeric convention).
class SimpleParameter : public Parameter
{
	public:
		// Type constructor
		SimpleParameter(SubatomicType type, unsigned int divisor = 1);
		// Copy constructor
		SimpleParameter(const SimpleParameter &copy);

		// as_simple_parameter returns the same parameter pointer converted to a simple parameter,
		//     if this is in fact a simple parameter; otherwise, returns NULL.
		virtual SimpleParameter *as_simple_parameter();
		virtual const SimpleParameter *as_simple_parameter() const;

		virtual Parameter *make_copy() const;
		virtual bool is_valid() const;

		SubatomicType get_type() const;
		bool has_modulus() const;
		double get_modulus() const;
		int get_divisor() const;

		bool is_numeric_type() const;
		bool set_modulus(double modulus);
		bool set_divisor(unsigned int divisor);
		bool set_range(const DoubleRange &range);

		virtual int calc_num_nested_fields(size_t length_bytes) const;
		virtual PackerInterface *get_nested_field(int n) const;

		virtual void pack_double(PackData &pack_data, double value,
		                         bool &pack_error, bool &range_error) const;
		virtual void pack_int(PackData &pack_data, int value,
		                      bool &pack_error, bool &range_error) const;
		virtual void pack_uint(PackData &pack_data, unsigned int value,
		                       bool &pack_error, bool &range_error) const;
		virtual void pack_int64(PackData &pack_data, int64_t value,
		                        bool &pack_error, bool &range_error) const;
		virtual void pack_uint64(PackData &pack_data, uint64_t value,
		                         bool &pack_error, bool &range_error) const;
		virtual void pack_string(PackData &pack_data, const std::string &value,
		                         bool &pack_error, bool &range_error) const;
		virtual bool pack_default_value(PackData &pack_data, bool &pack_error) const;

		virtual void unpack_double(const char *data, size_t length, size_t &p,
		                           double &value, bool &pack_error, bool &range_error) const;
		virtual void unpack_int(const char *data, size_t length, size_t &p,
		                        int &value, bool &pack_error, bool &range_error) const;
		virtual void unpack_uint(const char *data, size_t length, size_t &p,
		                         unsigned int &value, bool &pack_error, bool &range_error) const;
		virtual void unpack_int64(const char *data, size_t length, size_t &p,
		                          int64_t &value, bool &pack_error, bool &range_error) const;
		virtual void unpack_uint64(const char *data, size_t length, size_t &p,
		                           uint64_t &value, bool &pack_error, bool &range_error) const;
		virtual void unpack_string(const char *data, size_t length, size_t &p,
		                           std::string &value, bool &pack_error, bool &range_error) const;
		virtual bool unpack_validate(const char *data, size_t length, size_t &p,
		                             bool &pack_error, bool &range_error) const;
		virtual bool unpack_skip(const char *data, size_t length, size_t &p,
		                         bool &pack_error) const;

		virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                             const std::string &name, const std::string &postname) const;
		virtual void generate_hash(HashGenerator &hashgen) const;

	protected:
		virtual bool do_check_match(const PackerInterface *other) const;
		virtual bool do_check_match_simple_parameter(const SimpleParameter *other) const;
		virtual bool do_check_match_array_parameter(const ArrayParameter *other) const;

	private:
		static SimpleParameter *create_nested_field(SubatomicType type,
		        unsigned int divisor);
		static PackerInterface *create_uint32uint8_type();

	private:
		SubatomicType m_type;
		unsigned int m_divisor;

		SubatomicType m_nested_type;
		PackerInterface *m_nested_field;
		size_t m_bytes_per_element;

		// The rest of this is to maintain the static list of
		// PackerInterface objects for _nested_field, above.  We allocate
		// each possible object once, and don't delete it.
		static std::map<SubatomicType, std::map<unsigned int, SimpleParameter*> > cls_nested_field_map;

		// These are the range and modulus values as specified by the user,
		// unscaled by the divisor.
		DoubleRange m_orig_range;
		bool m_has_modulus;
		double m_orig_modulus;

		// Only the range appropriate to this type will be filled in.
		IntRange m_int_range;
		UnsignedIntRange m_uint_range;
		Int64Range m_int64_range;
		UnsignedInt64Range m_uint64_range;
		DoubleRange m_double_range;

		// All of these modulus values will be filled in, regardless of the
		// type.
		unsigned int m_uint_modulus;
		uint64_t m_uint64_modulus;
		double m_double_modulus;

		static ClassParameter *cls_uint32uint8_type;
};


} // close namespace dclass
