// Filename: Packer.h
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
#include "PackerInterface.h"
#include "SubatomicType.h"
#include "PackData.h"
#include "PackerCatalog.h"
#include <assert.h>
namespace dclass   // open namespace dclass
{


class Class;

////////////////////////////////////////////////////////////////////
//       Class : Packer
// Description : This class can be used for packing a series of
//               numeric and string data into a binary stream,
//               according to the  specification.
//
//               See also direct/src/doc/dcPacker.txt for a more
//               complete description and examples of using this
//               class.
////////////////////////////////////////////////////////////////////
class Packer
{
	public:
		Packer();
		~Packer();

		inline void clear_data();

		void begin_pack(const PackerInterface *root);
		bool end_pack();

		void set_unpack_data(const std::string &data);
		void set_unpack_data(const char *unpack_data, size_t unpack_length,
		                     bool owns_unpack_data);

		void begin_unpack(const PackerInterface *root);
		bool end_unpack();

		void begin_repack(const PackerInterface *root);
		bool end_repack();

		bool seek(const std::string &field_name);
		bool seek(int seek_index);

		inline bool has_nested_fields() const;
		inline int get_num_nested_fields() const;
		inline bool more_nested_fields() const;

		inline const PackerInterface *get_current_parent() const;
		inline const PackerInterface *get_current_field() const;
		inline PackType get_pack_type() const;
		inline std::string get_current_field_name() const;

		void push();
		void pop();

		inline void pack_double(double value);
		inline void pack_int(int value);
		inline void pack_uint(unsigned int value);
		inline void pack_int64(int64_t value);
		inline void pack_uint64(uint64_t value);
		inline void pack_string(const std::string &value);
		inline void pack_literal_value(const std::string &value);
		void pack_default_value();

		inline double unpack_double();
		inline int unpack_int();
		inline unsigned int unpack_uint();
		inline int64_t unpack_int64();
		inline uint64_t unpack_uint64();
		inline std::string unpack_string();
		inline std::string unpack_literal_value();
		void unpack_validate();
		void unpack_skip();

		// The following are variants on the above unpack() calls that pass
		// the result back by reference instead of as a return value.
		inline void unpack_double(double &value);
		inline void unpack_int(int &value);
		inline void unpack_uint(uint &value);
		inline void unpack_int64(int64_t &value);
		inline void unpack_uint64(uint64_t &value);
		inline void unpack_string(std::string &value);
		inline void unpack_literal_value(std::string &value);

		bool parse_and_pack(const std::string &formatted_object);
		bool parse_and_pack(std::istream &in);
		std::string unpack_and_format(bool show_field_names = true);
		void unpack_and_format(std::ostream &out, bool show_field_names = true);

		inline bool had_parse_error() const;
		inline bool had_pack_error() const;
		inline bool had_range_error() const;
		inline bool had_error() const;
		inline size_t get_num_unpacked_bytes() const;

		inline size_t get_length() const;
		inline std::string get_string() const;
		inline size_t get_unpack_length() const;
		inline std::string get_unpack_string() const;
		inline void get_string(std::string &data) const;
		inline const char *get_data() const;
		inline char *take_data();

		inline void append_data(const char *buffer, size_t size);
		inline char *get_write_pointer(size_t size);

		inline const char *get_unpack_data() const;

		inline static int get_num_stack_elements_ever_allocated();

		// The following methods are used only for packing (or unpacking)
		// raw data into the buffer between packing sessions (e.g. between
		// calls to end_pack() and the next begin_pack()).

		inline void raw_pack_int8(int8_t value);
		inline void raw_pack_int16(int16_t value);
		inline void raw_pack_int32(int32_t value);
		inline void raw_pack_int64(int64_t value);
		inline void raw_pack_uint8(uint8_t value);
		inline void raw_pack_uint16(uint16_t value);
		inline void raw_pack_uint32(uint32_t value);
		inline void raw_pack_uint64(uint64_t value);
		inline void raw_pack_float64(double value);
		inline void raw_pack_length_tag(length_tag_t value);
		inline void raw_pack_string(const std::string &value);

// this is a hack to allw me to get in and out of 32bit Mode Faster
// need to agree with channel_type in dcbase.h
#define RAW_PACK_CHANNEL(in)  raw_pack_uint64(in)
#define RAW_UNPACK_CHANNEL()  raw_unpack_uint64()


		inline int8_t raw_unpack_int8();
		inline int16_t raw_unpack_int16();
		inline int32_t raw_unpack_int32();
		inline int64_t raw_unpack_int64();
		inline uint8_t raw_unpack_uint8();
		inline uint16_t raw_unpack_uint16();
		inline uint32_t raw_unpack_uint32();
		inline uint64_t raw_unpack_uint64();
		inline double raw_unpack_float64();
		inline length_tag_t raw_unpack_length_tag();
		inline std::string raw_unpack_string();

	public:
		inline void raw_unpack_int8(int8_t &value);
		inline void raw_unpack_int16(int16_t &value);
		inline void raw_unpack_int32(int32_t &value);
		inline void raw_unpack_int64(int64_t &value);
		inline void raw_unpack_uint8(uint8_t &value);
		inline void raw_unpack_uint16(uint16_t &value);
		inline void raw_unpack_uint32(uint32_t &value);
		inline void raw_unpack_uint64(uint64_t &value);
		inline void raw_unpack_float64(double &value);
		inline void raw_unpack_length_tag(length_tag_t &value);
		inline void raw_unpack_string(std::string &value);

	public:
		static void enquote_string(std::ostream &out, char quote_mark, const std::string &str);
		static void output_hex_string(std::ostream &out, const std::string &str);

	private:
		inline void advance();
		void clear();
		void clear_stack();

	private:
		enum Mode
		{
		    M_idle,
		    M_pack,
		    M_unpack,
		    M_repack,
		};
		Mode _mode;

		PackData _pack_data;
		const char *_unpack_data;
		size_t _unpack_length;
		bool _owns_unpack_data;
		size_t _unpack_p;

		const PackerInterface *_root;
		const PackerCatalog *_catalog;
		const PackerCatalog::LiveCatalog *_live_catalog;

		class StackElement
		{
			public:
				// As an optimization, we implement operator new and delete here
				// to minimize allocation overhead during push() and pop().
				inline void *operator new(size_t size);
				inline void operator delete(void *ptr);

				const PackerInterface *_current_parent;
				int _current_field_index;
				size_t _push_marker;
				size_t _pop_marker;
				StackElement *_next;

				static StackElement *_deleted_chain;
				static int _num_ever_allocated;
		};
		StackElement *_stack;

		const PackerInterface *_current_field;
		const PackerInterface *_current_parent;
		int _current_field_index;

		// _push_marker marks the beginning of the push record (so we can go
		// back and write in the length later.
		size_t _push_marker;
		// _pop_marker is used in unpack mode with certain data structures
		// (like dynamic arrays) to mark the end of the push record (so we
		// know when we've reached the end).  It is zero when it is not in
		// use.
		size_t _pop_marker;
		int _num_nested_fields;

		bool _parse_error;
		bool _pack_error;
		bool _range_error;
};


} // close namespace dclass

#include "Packer.ipp"
