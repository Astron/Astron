// Filename: dcPackData.h
// Created by:  drose (15Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef DCPACKDATA_H
#define DCPACKDATA_H

#include "dcbase.h"

////////////////////////////////////////////////////////////////////
//       Class : DCPackData
// Description : This is a block of data that receives the results of
//               DCPacker.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCPackData
{
	PUBLISHED:
		inline DCPackData();
		inline ~DCPackData();

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
		char *_buffer;
		size_t _allocated_size;
		size_t _used_length;
};

#include "dcPackData.I"

#endif
