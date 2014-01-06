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
#include <iostream>
#include <string>

void init_file_lexer(std::istream &in, const std::string &filename);
void init_value_lexer(std::istream &in, const std::string &filename);

int run_lexer();

// we always read files
#define YY_NEVER_INTERACTIVE 1
