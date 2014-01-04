// Filename: indent.h
// Created by: drose (16 Jan, 1999)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include <iostream>
// indent outputs the indicated number of spaces to the given output stream, returning the
//     stream itself.  Useful for indenting a series of lines of text by a given amount.
std::ostream &indent(std::ostream &out, int indent_level);
