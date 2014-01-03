// Filename: parse.cpp
#include "parse.h"
#include "../DistributedType.h"
#include "../Struct.h"
#include "../Field.h"
#include "../AtomicField.h"
#include "../MolecularField.h"
#include "../Parameter.h"
#include "../file/ParserDefs.h"
#include <sstream>  // std::istringstream
using namespace std;
namespace dclass   // open namespace dclass
{


// parse_value reads a .dc-formatted parameter value and outputs the data in packed form matching
//     the appropriate DistributedType and suitable for a default parameter value.
string parse_value(const DistributedType* dtype, const string &formatted)
{
	istringstream strm(formatted);
	return parse_value(dtype, formatted);

}
string parse_value(const DistributedType* dtype, istream &in)
{
	string value;
	switch(dtype->get_datatype())
	{
		case DT_method:
		{
			// TODO: Update Atomic and Molecular heirarchy so I dont have to be silly like this
			const Field* field = dtype->as_field();
			if(field->as_molecular_field())
			{
				const MolecularField* molecular = field->as_molecular_field();
				size_t num_fields = molecular->get_num_atomics();
				for(unsigned int i = 0; i < num_fields; ++i)
				{
					value += parse_value(molecular->get_atomic(i), in);
				}
			}
			else if(field->as_atomic_field())
			{
				const AtomicField* atomic = atomic->as_atomic_field();
				size_t num_params = atomic->get_num_elements();
				for(unsigned int i = 0; i < num_params; ++i)
				{
					value += parse_value(atomic->get_element(i), in);
				}
			}
			break;
		}
		case DT_struct:
		{
			const Struct* dstruct = dtype->as_struct();
			size_t num_fields = dstruct->get_num_fields();
			for(unsigned int i = 0; i < num_fields; ++i)
			{
				value += parse_value(dstruct->get_field(i), in);
			}
		}
		default:
		{
			// TODO: Update Parameter/Field heirarchy so I don't have to be silly like this
			const Parameter* param = dtype->as_field()->as_parameter();
			dc_init_value_parser(in, "parse_value()", param, value);
			dcyyparse();
			dc_cleanup_parser();
		}
	}

	return value;
}


} // close namespace dclass
