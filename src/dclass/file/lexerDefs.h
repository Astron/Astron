// Filename: lexerDefs.h
#pragma once
#include <iostream>
#include <string>
namespace dclass   // open namespace dclass
{


void init_file_lexer(std::istream &in, const std::string &filename);
void init_value_lexer(std::istream &in, const std::string &filename);

int run_lexer();

int lexer_error_count();
int lexer_warning_count();

void lexer_error(const std::string &msg);
void lexer_warning(const std::string &msg);

// we always read files
#define YY_NEVER_INTERACTIVE 1


} // close namespace dclass
