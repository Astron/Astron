// Filename: LexerDefs.h
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

void dc_init_lexer(istream &in, const std::string &filename);
void dc_start_parameter_value();
void dc_start_parameter_description();
int dc_error_count();
int dc_warning_count();

void dcyyerror(const string &msg);
void dcyywarning(const string &msg);

int dcyylex();

// we always read files
#define YY_NEVER_INTERACTIVE 1
