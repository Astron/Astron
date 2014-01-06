// Filename: ParserDefs.h
#pragma once
#include "NumericRange.h"
#include <string>
namespace dclass   // open namespace dclass
{

// Foward declarations
class File;

class DistributedType;
class Struct;
class Class;
class Field;
class Method;
class Parameter;

// init_file_parser sets up a parsing session for reading an entire .dc file
void init_file_parser(std::istream &in, const std::string &filename, File &file);
// init_value_parser sets up a parsing session for reading a field or parameter value
void init_value_parser(std::istream &in, const std::string &filename,
                       const Parameter* param, std::string &output);
// run_parser runs the current parsing session
int run_parser();
// cleanup_parser cleans up after the last parsing session
void cleanup_parser();

int parser_error_count();
int parser_warning_count();

void parser_error(const std::string &msg);
void parser_warning(const std::string &msg);

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
      Class *dc_class;
      Struct *dc_struct;
      Field *dc_field;
      Method *dc_method;
      Parameter *dc_param;
      DistributedType* *dc_type;
    } u;
    std::string str;
    dclass::NumericRange range;
};

// The bison-generated code expects to use the symbol 'YYSTYPE' to refer to the above class.
#define YYSTYPE TokenType

} // close namespace dclass
