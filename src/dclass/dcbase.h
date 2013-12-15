// Filename: dcbase.h
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//


// Define a macro for assert variables when building for "Production"
#define _used_in_assert(x) ((void)x)

#include <stdint.h>
#include <string> // for std::string
#include <iostream> // for std::ostream, std::istream
#ifdef DCPARSER_32BIT_LENGTH_TAG
	typedef uint32_t length_tag_t;
#else
	typedef uint16_t length_tag_t;
#endif