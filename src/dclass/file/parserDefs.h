// Filename: parserDefs.h
#pragma once
#include <string> // std::string
#include <vector> // std::vector
#include "dc/DistributedType.h"
#include "dc/NumericRange.h"
namespace dclass   // open namespace dclass
{

// Foward declarations
class File;
class Class;
class Field;
class MolecularField;
class Parameter;

// init_file_parser sets up a parsing session for reading an entire .dc file
void init_file_parser(std::istream &in, const std::string &filename, File &file);
// init_value_parser sets up a parsing session for reading a field or parameter value
void init_value_parser(std::istream &in, const std::string &filename,
                       const DistributedType* dtype, std::string &output);
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

    union U {
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
        Type type;

        Class *dclass;
        Struct *dstruct;
        Field *dfield;
        MolecularField *dmolecule;
        Method *dmethod;
        Parameter *dparam;
        DistributedType* dtype;
        NumericType* dnumeric;
    } u;
    std::string str;
    std::vector<std::string> strings;
    NumericRange range;
    struct NameType {
        std::string name;
        DistributedType* type;
    } nametype;
};

// The bison-generated code expects to use the symbol 'YYSTYPE' to refer to the above class.
#ifndef YYSTYPE
#define YYSTYPE dclass::TokenType
#endif
extern YYSTYPE yylval;

} // close namespace dclass
