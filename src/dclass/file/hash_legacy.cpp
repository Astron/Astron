// Filename: hash.cpp
#include "hash.h"
#include "util/HashGenerator.h"

#include <set>      // std::set
#include <iostream> // std::cerr
#include <math.h>
#include "dc/File.h"
#include "dc/Class.h"
#include "dc/Method.h"
#include "dc/Parameter.h"
#include "dc/MolecularField.h"
#include "dc/NumericRange.h"
#include "dc/ArrayType.h"
#include "dc/NumericType.h"

using namespace std;
namespace dclass   // open namespace dclass
{


static void hash_file(HashGenerator& hashgen, const File* file);
static void hash_class(HashGenerator& hashgen, const Class* cls);
static void hash_struct(HashGenerator& hashgen, const Struct* strct);
static void hash_field(HashGenerator& hashgen, const Field* field);
static void hash_parameter(HashGenerator& hashgen, const Parameter* param);
static void hash_keywords(HashGenerator& hashgen, const KeywordList* list);
static void hash_legacy_type(HashGenerator& hashgen, const DistributedType* type);
static void hash_int_type(HashGenerator& hashgen, const NumericType* type);

// legacy_hash produces a hash which matches that of the old dcparser.
uint32_t legacy_hash(const File* file)
{
    HashGenerator hashgen;
    hash_file(hashgen, file);
    return hashgen.get_hash();
}

enum LegacyType {
    L_INT8,
    L_INT16,
    L_INT32,
    L_INT64,

    L_UINT8,
    L_UINT16,
    L_UINT32,
    L_UINT64,

    L_FLOAT64,

    L_STRING,
    L_BLOB,

    L_CHAR = 19,

    L_INVALID = 20
};

void hash_file(HashGenerator& hashgen, const File* file)
{
    hashgen.add_int(1); // (dc_virtual_inheritance && dc_sort_inheritance_by_file)
    hashgen.add_int(file->get_num_structs() + file->get_num_classes());

    size_t num_types = file->get_num_types();
    for(size_t i{}; i < num_types; ++i) {
        const DistributedType* type = file->get_type_by_id(i);
        if(!type->as_struct()) {
            cerr << "Cannot generate legacy hash for this file.\n";
            return;
        }

        const Struct* strct = type->as_struct();
        if(strct->as_class()) {
            hash_class(hashgen, strct->as_class());
        } else {
            hash_struct(hashgen, strct);
        }
    }
}

void hash_class(HashGenerator& hashgen, const Class* cls)
{
    hashgen.add_string(cls->get_name());

    size_t num_parents = cls->get_num_parents();
    hashgen.add_int(num_parents);
    for(size_t i{}; i < num_parents; ++i) {
        hashgen.add_int(cls->get_parent(i)->get_id());
    }

    if(cls->has_constructor()) {
        hash_field(hashgen, cls->get_constructor());
    }

    size_t num_fields = cls->get_num_base_fields();
    hashgen.add_int(num_fields);
    for(size_t i{}; i < num_fields; ++i) {
        hash_field(hashgen, cls->get_base_field(i));
    }
}

void hash_struct(HashGenerator& hashgen, const Struct* strct)
{
    hashgen.add_string(strct->get_name());
    hashgen.add_int(1); // is_struct()
    hashgen.add_int(0); // get_num_parents()

    size_t num_fields = strct->get_num_fields();
    hashgen.add_int(num_fields);
    for(size_t i{}; i < num_fields; ++i) {
        hash_field(hashgen, strct->get_field(i));
    }
}

void hash_field(HashGenerator& hashgen, const Field* field)
{
    /* Handle DCMolecularField */
    if(field->as_molecular()) {
        // DCField::generate_hash()
        hashgen.add_string(field->get_name());
        hashgen.add_int(field->get_id());

        const MolecularField* mol = field->as_molecular();
        size_t num_fields = mol->get_num_fields();

        hashgen.add_int(num_fields); // _fields.size();
        for(size_t i{}; i < num_fields; ++i) {
            hash_field(hashgen, mol->get_field(i));
        }
        return;
    }


    /* Handle DCAtomicField */
    if(field->get_type()->get_type() == T_METHOD) {
        // DCField::generate_hash()
        hashgen.add_string(field->get_name());
        hashgen.add_int(field->get_id());

        const Method* method = field->get_type()->as_method();
        size_t num_params = method->get_num_parameters();

        hashgen.add_int(num_params); // _elements.size();
        for(size_t i{}; i < num_params; ++i) {
            hash_parameter(hashgen, method->get_parameter(i));
        }

        // DCKeywordList::generate_hash()
        hash_keywords(hashgen, field);
        return;
    }


    /* Handle DCSimpleParameter, DCClassParameter, DCArrayParameter */
    // DCParameter::generate_hash()
    if(field->get_num_keywords() != 0) {
        // DCKeywordList::generate_hash()
        hash_keywords(hashgen, field);
    }
    hash_legacy_type(hashgen, field->get_type());
}

void hash_parameter(HashGenerator& hashgen, const Parameter* param)
{
    hash_legacy_type(hashgen, param->get_type());
}

void hash_keywords(HashGenerator& hashgen, const KeywordList* list)
{
    struct LegacyKeyword {
        const char* keyword;
        int flag;
    };
    static LegacyKeyword legacy_keywords[] = {
        { "required", 0x0001 },
        { "broadcast", 0x0002 },
        { "ownrecv", 0x0004 },
        { "ram", 0x0008 },
        { "db", 0x0010 },
        { "clsend", 0x0020 },
        { "clrecv", 0x0040 },
        { "ownsend", 0x0080 },
        { "airecv", 0x0100 },
        { nullptr, 0 }
    };

    size_t num_keywords = list->get_num_keywords();

    int flags = 0;
    for(size_t i{}; i < num_keywords; ++i) {
        bool set_flag = false;
        string keyword = list->get_keyword(i);
        for(size_t j{}; j < sizeof(legacy_keywords); ++j) {
            if(keyword == legacy_keywords[j].keyword) {
                flags |= legacy_keywords[j].flag;
                set_flag = true;
                break;
            }
        }

        if(!set_flag) {
            flags = ~0;
            break;
        }
    }

    if(flags != ~0) {
        hashgen.add_int(flags);
    } else {
        hashgen.add_int(num_keywords); // _keywords_by_name.size()

        set<string> keywords_by_name;
        for(size_t i{}; i < num_keywords; ++i) {
            keywords_by_name.insert(list->get_keyword(i));
        }

        for(const auto& it : keywords_by_name) {
            // keyword->generate_hash();
            hashgen.add_string(it);
        }
    }
}

void hash_legacy_type(HashGenerator& hashgen, const DistributedType* type)
{
    switch(type->get_type()) {
    case T_STRUCT: {
        // get_class()->generate_hash()
        const Struct* strct = type->as_struct();
        if(strct->as_class()) {
            hash_class(hashgen, strct->as_class());
        } else {
            hash_struct(hashgen, strct);
        }
        break;
    }

    case T_ARRAY:
    case T_VARARRAY: {
        const ArrayType* arr = type->as_array();

        // _element_type->generate_hash()
        hash_legacy_type(hashgen, arr->get_element_type());

        // _array_size_range.generate_hash()
        if(arr->has_range()) {
            NumericRange rng = arr->get_range();
            hashgen.add_int(1); // _range._ranges.size();
            hashgen.add_int((int)rng.min.uinteger);
            hashgen.add_int((int)rng.max.uinteger);
        }
        break;
    }

    case T_BLOB:
    case T_VARBLOB: {
        if(type->get_alias() == "blob") {
            hashgen.add_int(L_BLOB); // _type
        } else {
            hashgen.add_int(L_UINT8); // _type
        }
        hashgen.add_int(1); // _divisor

        const ArrayType* blob = type->as_array();
        if(blob->has_range()) {
            // _uint_range.generate_hash();
            NumericRange rng = blob->get_range();
            hashgen.add_int(1); // _range._ranges.size();
            hashgen.add_int((int)rng.min.uinteger);
            hashgen.add_int((int)rng.max.uinteger);
        }
        break;
    }

    case T_VARSTRING:
    case T_STRING: {
        if(type->get_alias() == "string") {
            hashgen.add_int(L_STRING); // _type
        } else {
            hashgen.add_int(L_CHAR); // _type
        }
        hashgen.add_int(1); // _divisor

        const ArrayType* str = type->as_array();
        if(str->has_range()) {
            // _uint_range.generate_hash());
            NumericRange rng = str->get_range();
            hashgen.add_int(1u); // _range._ranges.size();
            hashgen.add_int((int)rng.min.uinteger);
            hashgen.add_int((int)rng.max.uinteger);
        }
        break;
    }

    case T_INT8:
        hashgen.add_int(L_INT8);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_INT16:
        hashgen.add_int(L_INT16);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_INT32:
        hashgen.add_int(L_INT32);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_INT64:
        hashgen.add_int(L_INT64);
        hash_int_type(hashgen, type->as_numeric());
        break;

    case T_UINT8:
        hashgen.add_int(L_UINT8);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_UINT16:
        hashgen.add_int(L_UINT16);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_UINT32:
        hashgen.add_int(L_UINT32);
        hash_int_type(hashgen, type->as_numeric());
        break;
    case T_UINT64:
        hashgen.add_int(L_UINT64);
        hash_int_type(hashgen, type->as_numeric());
        break;

    case T_CHAR:
        hashgen.add_int(L_CHAR);
        hash_int_type(hashgen, type->as_numeric());
        break;

    case T_FLOAT64: {
        hashgen.add_int(L_FLOAT64); // _type

        const NumericType* numeric = type->as_numeric();
        hashgen.add_int(numeric->get_divisor()); // _divisor
        if(numeric->has_modulus()) {
            hashgen.add_int(int(numeric->get_modulus() * numeric->get_divisor())); // _modulus
        }
        if(numeric->has_range()) {
            // _double_range.generate_hash());
            NumericRange rng = numeric->get_range();
            hashgen.add_int(1u); // _range._ranges.size();
            hashgen.add_int(int(rng.min.floating * numeric->get_divisor()));
            hashgen.add_int(int(rng.max.floating * numeric->get_divisor()));
        }
        break;
    }

    case T_FLOAT32:
        cerr << "Warning: float32 ignored in legacy_hash.\n";
        break;

    case T_INVALID:
        cerr << "Error: Cannot generate legacy_hash, encountered invalid type.\n";
        break;

    default:
        cerr << "Error: Unexpected type in hash_legacy_type while generating legacy_hash.\n";
    }
}


void hash_int_type(HashGenerator& hashgen, const NumericType* numeric)
{
    hashgen.add_int(numeric->get_divisor());
    if(numeric->has_modulus()) {
        unsigned int modulus = (unsigned int)floor(numeric->get_modulus() * numeric->get_divisor() + 0.5);
        hashgen.add_int(int(modulus));
    }
    if(numeric->has_range()) {
        // _uint_range.generate_hash());
        NumericRange rng = numeric->get_range();
        hashgen.add_int(1u); // _range._ranges.size();
        hashgen.add_int(int(floor(rng.min.floating * numeric->get_divisor() + 0.5)));
        hashgen.add_int(int(floor(rng.max.floating * numeric->get_divisor() + 0.5)));
    }
}


} // close namespace dclass
