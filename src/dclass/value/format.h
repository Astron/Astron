// Filename: format.h
#pragma once
#include <string> // std::string
#include <vector> // std::vector
namespace dclass   // open namespace dclass
{


// Forward declarations
class DistributedType;

// format_value steps through packed data and unpacks it as a .dc file parameter value.
//     An DistributedType represents any type with defined structure (Class, Field, method, etc...)
std::string format_value(const DistributedType*, const std::vector<uint8_t> &packed);
std::string format_value(const DistributedType*, const std::string &packed);
void format_value(const DistributedType*, const std::vector<uint8_t> &packed, std::ostream &out);
void format_value(const DistributedType*, const std::string &packed, std::ostream &out);

// format_hex outputs <str> as a hexidecimal constant enclosed in angle-brackets (<>)
std::string format_hex(const std::string &str);
void format_hex(const std::string &str, std::ostream &out);

// format_quoted outputs <str> enclosed in quotes after escaping the string.
//     Any instances of backslash (\) or the quoute character in the string are escaped.
//     Non-printable characters are replaced with an escaped hexidecimal constant.std::string format_quoted(const std::string &str);
std::string format_quoted(char quote_mark, const std::string &str);
void format_quoted(char quote_mark, const std::string &str, std::ostream &out);


} // close namespace dclass
