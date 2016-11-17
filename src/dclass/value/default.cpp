// Filename: default.cpp
#include <stdint.h> // fixed-width integer types
#include "dc/ArrayType.h"
#include "dc/Struct.h"
#include "dc/Field.h"
#include "dc/Method.h"
#include "dc/Parameter.h"

#include "default.h"
using namespace std;
namespace dclass   // open namespace dclass
{


// create_default_value returns a sensible default value for the given type (typically 0).
string create_default_value(const DistributedType* dtype)
{
    bool discard;
    return create_default_value(dtype, discard);
}

// create_default_value returns a sensible default value for the given type (typically 0).
//     is_implicit will be true if all component values are implicit,
//     or false otherwise (ie. at least one value was user-defined via set_default_value).
string create_default_value(const DistributedType* dtype, bool& is_implicit)
{
    if(dtype == nullptr) {
        return "";
    }

    // Unless we found a non-implicit value, we are an implicit value
    is_implicit = true;
    switch(dtype->get_type()) {
    case T_INT8: {
        int8_t v = 0;
        return string((char*)&v, sizeof(int8_t));
    }
    case T_INT16: {
        uint16_t v = 0;
        return string((char*)&v, sizeof(int16_t));
    }
    case T_INT32: {
        int32_t v = 0;
        return string((char*)&v, sizeof(int32_t));
    }
    case T_INT64: {
        int64_t v = 0;
        return string((char*)&v, sizeof(int64_t));
    }
    case T_CHAR:
    case T_UINT8: {
        uint8_t v = 0;
        return string((char*)&v, sizeof(uint8_t));
    }
    case T_UINT16: {
        uint16_t v = 0;
        return string((char*)&v, sizeof(uint16_t));
    }
    case T_UINT32: {
        uint32_t v = 0;
        return string((char*)&v, sizeof(uint32_t));
    }
    case T_UINT64: {
        uint64_t v = 0;
        return string((char*)&v, sizeof(uint64_t));
    }
    case T_FLOAT32: {
        float v = 0.0f;
        return string((char*)&v, sizeof(float));
    }
    case T_FLOAT64: {
        double v = 0.0;
        return string((char*)&v, sizeof(double));
    }
    case T_ARRAY:
    case T_BLOB:
    case T_STRING: {
        const ArrayType* array = dtype->as_array();
        uint64_t min_array_elements = array->get_range().min.uinteger;
        return string((unsigned int)min_array_elements, '\0');
    }
    case T_VARARRAY:
    case T_VARBLOB:
    case T_VARSTRING: {
        const ArrayType* array = dtype->as_array();
        sizetag_t len = (sizetag_t)array->get_range().min.uinteger;
        return string((char*)&len, sizeof(sizetag_t)) + string(len, '\0');
    }
    case T_STRUCT: {
        const Struct* dstruct = dtype->as_struct();
        size_t num_fields = dstruct->get_num_fields();

        string val;
        for(unsigned int i = 0; i < num_fields; ++i) {
            const Field* field = dstruct->get_field(i);
            if(field->has_default_value()) {
                is_implicit = false;
            }
            val += field->get_default_value();
        }
        return val;
    }
    case T_METHOD: {
        const Method* dmethod = dtype->as_method();
        size_t num_params = dmethod->get_num_parameters();

        string val;
        for(unsigned int i = 0; i < num_params; ++i) {
            const Parameter* param = dmethod->get_parameter(i);
            if(param->has_default_value()) {
                is_implicit = false;
            }
            val += param->get_default_value();
        }
        return val;
    }
    default: {
        return string();
    }
    }
}


} // close namespace dclass
