// Filename: default.cpp
#include "value/default.h"
#include "Struct.h"
#include "ArrayType.h"
#include "Method.h"
#include "Field.h"
#include "Parameter.h"
using namespace std;
namespace dclass   // open namespace dclass
{


// create_default_value returns a sensible default value for the given type (typically 0).
std::string create_default_value(const DistributedType* dtype)
{
	switch(dtype->get_type())
	{
		case INT8:
		{
			int8_t v = 0;
			return string((char*)&v, sizeof(int8_t));
		}
		case INT16:
		{
			uint16_t v = 0;
			return string((char*)&v, sizeof(int16_t));
		}
		case INT32:
		{
			int32_t v = 0;
			return string((char*)&v, sizeof(int32_t));
		}
		case INT64:
		{
			int64_t v = 0;
			return string((char*)&v, sizeof(int64_t));
		}
		case CHAR:
		case UINT8:
		{
			uint8_t v = 0;
			return string((char*)&v, sizeof(uint8_t));
		}
		case UINT16:
		{
			uint16_t v = 0;
			return string((char*)&v, sizeof(uint16_t));
		}
		case UINT32:
		{
			uint32_t v = 0;
			return string((char*)&v, sizeof(uint32_t));
		}
		case UINT64:
		{
			uint64_t v = 0;
			return string((char*)&v, sizeof(uint64_t));
		}
		case FLOAT32:
		{
			float v = 0.0f;
			return string((char*)&v, sizeof(float));
		}
		case FLOAT64:
		{
			double v = 0.0;
			return string((char*)&v, sizeof(double));
		}
		case ARRAY:
		case BLOB:
		case STRING:
		{
			const ArrayType* array = dtype->as_array();
			uint64_t min_array_elements = array->get_range().min.uinteger;
			return string(min_array_elements, '\0');
		}
		case VARARRAY:
		case VARBLOB:
		case VARSTRING:
		{
			const ArrayType* array = dtype->as_array();
			sizetag_t len = (sizetag_t)array->get_range().min.uinteger;
			return string((char*)&len, sizeof(sizetag_t)) + string(len, '\0');
		}
		case STRUCT:
		{
			const Struct* dstruct = dtype->as_struct();
			size_t num_fields = dstruct->get_num_fields();

			string val;
			for(unsigned int i = 0; i < num_fields; ++i)
			{
				val += create_default_value(dstruct->get_field(i)->get_type());
			}
			return val;
		}
		case METHOD:
		{
			const Method* dmethod = dtype->as_method();
			size_t num_params = dmethod->get_num_parameters();

			string val;
			for(unsigned int i = 0; i < num_params; ++i)
			{
				val += create_default_value(dmethod->get_parameter(i)->get_type());
			}
			return val;
		}
		default:
		{
			return string();
		}
	}
}


} // close namespace dclass