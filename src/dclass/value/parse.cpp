// Filename: parse.cpp
#include <sstream>  // std::istringstream
#include "dc/DistributedType.h"
#include "file/parserDefs.h"

#include "parse.h"
using namespace std;
namespace dclass   // open namespace dclass
{


// parse_value reads a .dc-formatted parameter value and outputs the data in packed form matching
//     the appropriate DistributedType and suitable for a default parameter value.
//     If an error occurs, the error reason is returned instead of the parsed value.
string parse_value(const DistributedType* dtype, const string &formatted, bool &err)
{
    istringstream strm(formatted);
    return parse_value(dtype, strm, err);

}
string parse_value(const DistributedType* dtype, istream &in, bool &err)
{
    string value;
    try {
        init_value_parser(in, "parse_value()", dtype, value);
        run_parser();
        cleanup_parser();
    } catch(const exception& e) {
        err = true;
        return string("parse_value() error: ") + e.what();
    }

    if(parser_error_count() > 0) {
        err = true;
        return string("parse value(): unknown error");
    }

    err = false;
    return value;
}


} // close namespace dclass
