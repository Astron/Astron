// Filename: dcbase.h
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//


#pragma once
#ifdef WIN32
	/* C4786: 255 char debug symbols */
	#pragma warning (disable : 4786)
	/* C4503: decorated name length exceeded */
	#pragma warning (disable : 4503)
#endif  /* WIN32_VC */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <string>
#include <assert.h>

// These header files are needed to compile dcLexer.cxx, the output
// from flex.  flex doesn't create a perfectly windows-friendly source
// file right out of the box.
#ifdef WIN32
	#include <io.h>
	#include <malloc.h>
#else
	#include <unistd.h>
#endif

using namespace std;


// These symbols are used within the Panda environment for exporting
// classes and functions to the scripting language.  They're largely
// meaningless if we're not compiling within Panda.
#define PUBLISHED public
#define BEGIN_PUBLISH
#define END_PUBLISH
#define BLOCKING

// Panda defines some assert-type macros.  We map those to the
// standard assert macro outside of Panda.
#define nassertr(condition, return_value) assert(condition)
#define nassertr_always(condition, return_value) assert(condition)
#define nassertv(condition) assert(condition)
#define nassertv_always(condition) assert(condition)
// Define a macro for assert variables when building for "Production"
#define _used_in_assert(x) ((void)x)

// Panda defines these export symbols for building DLL's.  Outside of
// Panda, we assume we're not putting this code in a DLL, so we define
// them to nothing.
#define EXPCL_DIRECT
#define EXPTP_DIRECT

// Panda defines a special Filename class.  We'll use an ordinary
// string instead.
typedef std::string Filename;

// Panda defines WORDS_BIGENDIAN on a bigendian machine; otherwise,
// the machine is assumed to be littleendian.  Outside of Panda,
// you're responsible for defining this yourself if necessary.
//#define WORDS_BIGENDIAN

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stdint.h>
#include <string.h>

#ifdef DCPARSER_32BIT_LENGTH_TAG
	typedef uint32_t length_tag_t;
#else
	typedef uint16_t length_tag_t;
#endif

// TODO: Channels, doids, and zones should not exist in the dcparser! The code that uses these
// belongs in another module (for example, within the CMU code, etc....)
//typedef       unsigned long   CHANNEL_TYPE;
typedef uint64_t CHANNEL_TYPE;
typedef uint32_t DOID_TYPE;
typedef uint32_t ZONEID_TYPE;
