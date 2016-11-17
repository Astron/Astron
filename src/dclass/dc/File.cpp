// Filename: File.cpp
#include "util/HashGenerator.h"
#include "dc/DistributedType.h"
#include "dc/Class.h"
#include "dc/Field.h"

#include "File.h"
namespace dclass   // open namespace
{


typedef std::unordered_map<std::string, DistributedType*>::value_type TypeName;

// constructor
File::File()
{
}

//destructor
File::~File()
{
    for(auto it = m_classes.begin(); it != m_classes.end(); ++it) {
        delete(*it);
    }
    for(auto it = m_structs.begin(); it != m_structs.end(); ++it) {
        delete(*it);
    }
    for(auto it = m_imports.begin(); it != m_imports.end(); ++it) {
        delete(*it);
    }

    m_classes.clear();
    m_structs.clear();
    m_imports.clear();
    m_types_by_id.clear();
    m_types_by_name.clear();
    m_fields_by_id.clear();
    m_keywords.clear();
}

// get_class_by_id returns the requested class or nullptr if there is no such class.
Class* File::get_class_by_id(unsigned int id)
{
    DistributedType* dt = get_type_by_id(id);
    if(dt == nullptr) {
        return nullptr;
    }
    if(dt->as_struct() == nullptr) {
        return nullptr;
    }
    return dt->as_struct()->as_class();
}
const Class* File::get_class_by_id(unsigned int id) const
{
    const DistributedType* dt = get_type_by_id(id);
    if(dt == nullptr) {
        return nullptr;
    }
    if(dt->as_struct() == nullptr) {
        return nullptr;
    }
    return dt->as_struct()->as_class();
}
// get_class_by_name returns the requested class or nullptr if there is no such class.
Class* File::get_class_by_name(const std::string &name)
{
    DistributedType* dt = get_type_by_name(name);
    if(dt == nullptr) {
        return nullptr;
    }
    if(dt->as_struct() == nullptr) {
        return nullptr;
    }
    return dt->as_struct()->as_class();
}
const Class* File::get_class_by_name(const std::string &name) const
{
    const DistributedType* dt = get_type_by_name(name);
    if(dt == nullptr) {
        return nullptr;
    }
    if(dt->as_struct() == nullptr) {
        return nullptr;
    }
    return dt->as_struct()->as_class();
}

// add_class adds the newly-allocated class to the file.
//     Returns false if there is a name conflict.
//     The File becomes the owner of the pointer and will delete it when it destructs.
bool File::add_class(Class *cls)
{
    // Classes have to have a name
    if(cls->get_name().empty()) {
        return false;
    }

    // A Class can't share a name with any other type.
    bool inserted = m_types_by_name.insert(TypeName(cls->get_name(), cls)).second;
    if(!inserted) {
        return false;
    }

    cls->set_id(m_types_by_id.size());
    m_types_by_id.push_back(cls);
    m_classes.push_back(cls);
    return true;
}

// add_struct adds the newly-allocated struct to the file.
//     Returns false if there is a name conflict.
//     The File becomes the owner of the pointer and will delete it when it destructs.
bool File::add_struct(Struct *strct)
{
    // Structs have to have a name
    if(strct->get_name().empty()) {
        return false;
    }

    // A Struct can't share a name with any other type.
    bool inserted = m_types_by_name.insert(TypeName(strct->get_name(), strct)).second;
    if(!inserted) {
        return false;
    }

    strct->set_id(m_types_by_id.size());
    m_types_by_id.push_back(strct);
    m_structs.push_back(strct);
    return true;
}

// add_typedef adds the alias <name> to the file for the type <type>.
//     Returns false if there is a name conflict.
bool File::add_typedef(const std::string& name, DistributedType* type)
{
    // Typedefs can't use the empty string as a name
    if(name.empty()) {
        return false;
    }

    // A type alias can't share a name with any other type.
    return m_types_by_name.insert(TypeName(name, type)).second;
}

// add_import adds a newly-allocated import to the file.
//     Imports with duplicate modules are combined.
void File::add_import(Import* import)
{
    // TODO: Combine duplicates
    m_imports.push_back(import);
}

// add_keyword adds a keyword with the name <keyword> to the list of declared keywords.
void File::add_keyword(const std::string &keyword)
{
    if(!has_keyword(keyword)) {
        m_keywords.push_back(keyword);
    }
}

// add_field gives the field a unique id within the file.
void File::add_field(Field *field)
{
    field->set_id((unsigned int)m_fields_by_id.size());
    m_fields_by_id.push_back(field);
}

uint32_t File::get_hash() const
{
    HashGenerator hashgen;
    generate_hash(hashgen);
    return hashgen.get_hash();
}

// generate_hash accumulates the properties of this file into the hash.
void File::generate_hash(HashGenerator& hashgen) const
{
    hashgen.add_int(m_classes.size());
    for(auto it = m_classes.begin(); it != m_classes.end(); ++it) {
        (*it)->generate_hash(hashgen);
    }

    hashgen.add_int(m_structs.size());
    for(auto it = m_structs.begin(); it != m_structs.end(); ++it) {
        (*it)->generate_hash(hashgen);
    }

    hashgen.add_int(m_keywords.size());
    for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it) {
        hashgen.add_string(*it);
    }
}


} // close namespace dclass
