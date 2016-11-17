// Filename: Struct.cpp
#include "util/HashGenerator.h"
#include "dc/File.h"
#include "dc/Field.h"

#include "Struct.h"
using namespace std;
namespace dclass   // open namespace dclass
{

// public constructor
Struct::Struct(File* file, const string& name) : m_file(file), m_id(0), m_name(name)
{
    m_type = T_STRUCT;
}

// protected constructor
Struct::Struct(File* file) : m_file(file), m_id(0)
{
    m_type = T_STRUCT;
}

// destructor
Struct::~Struct()
{
    for(auto& it : m_fields) {
        delete it;
    }
}

// as_struct returns this as a Struct if it is a Struct, or nullptr otherwise.
Struct* Struct::as_struct()
{
    return this;
}
const Struct* Struct::as_struct() const
{
    return this;
}

// as_class returns this Struct as a Class if it is a Class, or nullptr otherwise.
Class* Struct::as_class()
{
    return nullptr;
}
const Class* Struct::as_class() const
{
    return nullptr;
}

// add_field adds a new Field to the struct.
bool Struct::add_field(Field* field)
{
    // Field must not be null
    if(field == nullptr) {
        return false;
    }

    // Structs can't share a field
    if(field->get_struct() != nullptr && field->get_struct() != this) {
        return false;
    }

    // Structs can't have moleculars
    if(field->as_molecular()) {
        return false;
    }
    // Structs can't have methods
    if(field->get_type()->as_method()) {
        return false;
    }

    // Struct fields are accessible by name
    if(!field->get_name().empty()) {
        // Structs can't have Constructors
        if(field->get_name() == m_name) {
            return false;
        }

        // Try to add the field
        bool inserted = m_fields_by_name.insert(
                            unordered_map<string, Field*>::value_type(field->get_name(), field)).second;

        if(!inserted) {
            // But the field had a name conflict
            return false;
        }
    }

    // Struct fields are accessible by id.
    m_file->add_field(field);
    m_fields_by_id.insert(unordered_map<int, Field*>::value_type(field->get_id(), field));

    m_fields.push_back(field);
    if(has_fixed_size() || m_fields.size() == 1) {
        if(field->get_type()->has_fixed_size()) {
            m_size += field->get_type()->get_size();
        } else {
            m_size = 0;
        }
    }
    return true;
}

// generate_hash accumulates the properties of this class into the hash.
void Struct::generate_hash(HashGenerator& hashgen) const
{
    DistributedType::generate_hash(hashgen);
    hashgen.add_string(m_name);
    hashgen.add_int(m_fields.size());
    for(const auto& it : m_fields) {
        it->generate_hash(hashgen);
    }
}


} // close namespace dclass
