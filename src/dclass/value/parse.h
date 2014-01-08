// Filename: parse.h
#pragma once
#include <string> // std::string
#include <vector> // std::vector
namespace dclass   // open namespace dclass
{


// Forward declarations
class DistributedType;

// parse_value reads a .dc-formatted parameter value and outputs the data in packed form matching
//     the appropriate DistributedType and suitable for a default parameter value.
//     If an error occurs, the error reason is returned instead of the parsed value.
std::string parse_value(const DistributedType*, const std::string &formatted, bool &err);
std::string parse_value(const DistributedType*, std::istream &in, bool &err);


} // close namespace dclass
