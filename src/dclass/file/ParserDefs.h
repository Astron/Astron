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
#include "../DataType.h"
#include "../NumericRange.h"

namespace dclass
{
	class File;
	class Class;
  class Struct;
	class Field;
	class AtomicField;
	class Parameter;
	class Keyword;
}

void dc_init_parser(std::istream &in, const std::string &filename, dclass::File &file);
void dc_cleanup_parser();
int dcyyparse();

extern dclass::File *dc_file;

class TokenType
{

  public:
    union U
    {
      int8_t int8;
      int16_t int16;
      int32_t int32;
      int64_t int64;
      uint8_t uint8;
      uint16_t uint16;
      uint32_t uint32;
      uint64_t uint64;
      double real;
      bool flag;
      dclass::Class *dclass;
      dclass::Struct *dstruct;
      dclass::Field *field;
      dclass::AtomicField *atomic;
      dclass::DataType datatype;
      dclass::Parameter *parameter;
      const dclass::Keyword *keyword;
    } u;
    std::string str;
    dclass::NumericRange range;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to
// refer to the above class.
#define YYSTYPE TokenType