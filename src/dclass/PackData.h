// Filename: PackData.h
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
namespace dclass   // open namespace dclass
{


// A PackData is a block of data that receives the results of Packer.
class EXPCL_DIRECT PackData
{
	PUBLISHED:
		inline PackData();
		inline ~PackData();

		inline void clear();

	public:
		inline void append_data(const char *buffer, size_t size);
		inline char *get_write_pointer(size_t size);
		inline void append_junk(size_t size);
		inline void rewrite_data(size_t position, const char *buffer, size_t size);
		inline char *get_rewrite_pointer(size_t position, size_t size);

	PUBLISHED:
		inline string get_string() const;
		inline size_t get_length() const;
	public:
		inline const char *get_data() const;
		inline char *take_data();

	private:
		void set_used_length(size_t size);

	private:
		char *m_buffer;
		size_t m_allocated_size;
		size_t m_used_length;
};


} // close namespace dclass

#include "PackData.ipp"
