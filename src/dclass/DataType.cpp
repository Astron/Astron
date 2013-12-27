// Filename: DataType.cpp

#include "DataType.h"
namespace dclass   // open namespace dclass
{


std::ostream & operator << (std::ostream &out, DataType type)
{
	switch(type)
	{
		case DT_int8:
			return out << "int8";

		case DT_int16:
			return out << "int16";

		case DT_int32:
			return out << "int32";

		case DT_int64:
			return out << "int64";

		case DT_uint8:
			return out << "uint8";

		case DT_uint16:
			return out << "uint16";

		case DT_uint32:
			return out << "uint32";

		case DT_uint64:
			return out << "uint64";

		case DT_float32:
			return out << "float32";

		case DT_float64:
			return out << "float64";

		case DT_char:
			return out << "char";

		case DT_string:
			return out << "string";

		case DT_varstring:
			return out << "varstring";

		case DT_blob:
			return out << "blob";

		case DT_varblob:
			return out << "varblob";

		case DT_array:
			return out << "array";

		case DT_vararray:
			return out << "vararray";

		case DT_method:
			return out << "method";

		case DT_struct:
			return out << "struct";

		case DT_invalid:
			return out << "invalid";
	}

	return out << "invalid type: " << (int)type;
}


} // close namespace dclass
