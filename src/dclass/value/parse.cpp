// Filename: parse.cpp
#include "value/parse.h"
#include "file/parserDefs.h"
#include "DistributedType.h"
#include "Struct.h"
#include "Field.h"
#include "Parameter.h"
#include <sstream>  // std::istringstream
using namespace std;
namespace dclass   // open namespace dclass
{


// parse_value reads a .dc-formatted parameter value and outputs the data in packed form matching
//     the appropriate DistributedType and suitable for a default parameter value.
string parse_value(const DistributedType* dtype, const string &formatted)
{
	istringstream strm(formatted);
	return parse_value(dtype, strm);

}
string parse_value(const DistributedType* dtype, istream &in)
{
	string value;
	init_value_parser(in, "parse_value()", dtype, value);
	run_parser();
	cleanup_parser();
	return value;
}


} // close namespace dclass
