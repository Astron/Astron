// Filename: ParserDefs.h
// Created by: drose (05 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "SubatomicType.h"

namespace dclass
{
	class File;
	class Class;
	class Field;
	class AtomicField;
	class Parameter;
	class Keyword;
	class Packer;
}
using namespace dclass;

void dc_init_parser(std::istream &in, const std::string &filename, File &file);
void dc_init_parser_parameter_value(std::istream &in, const std::string &filename,
                                    Packer &packer);
void dc_init_parser_parameter_description(std::istream &in, const std::string &filename,
        File *file);
Field *dc_get_parameter_description();
void dc_cleanup_parser();
int dcyyparse();

extern File *dc_file;

// This structure holds the return value for each token.
// Traditionally, this is a union, and is declared with the %union
// declaration in the parser.y file, but unions are pretty worthless
// in C++ (you can't include an object that has member functions in a
// union), so we'll use a class instead.  That means we need to
// declare it externally, here.

class TokenType
{
	public:
		union U
		{
			int s_int;
			unsigned int s_uint;
			int64_t int64;
			uint64_t uint64;
			double real;
			bool flag;
			Class *dclass;
			Field *field;
			AtomicField *atomic;
			SubatomicType subatomic;
			Parameter *parameter;
			const Keyword *keyword;
		} u;
		std::string str;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to
// refer to the above class.
#define YYSTYPE TokenType
