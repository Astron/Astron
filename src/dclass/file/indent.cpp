// Filename: indent.cpp
// Created by: drose (05 May, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "indent.h"

std::ostream &indent(std::ostream &out, int indent_level)
{
	for(int i = 0; i < indent_level; i++)
	{
		out << ' ';
	}
	return out;
}