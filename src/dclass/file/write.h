// Filename: write.h
#pragma once
#include <iostream>
namespace dclass   // open namespace dclass
{


// indent outputs the indicated number of spaces to the given output stream, returning the
//     stream itself.  Useful for indenting a series of lines of text by a given amount.
std::ostream& indent(std::ostream& out, unsigned int indent_level);


} // close namespace dclass
