// Filename: File.h
#pragma once
#include <stdint.h>
#include <string>        // std::string
#include <vector>        // std::vector
#include <unordered_map> // std::unordered_map
namespace dclass   // open namespace
{


// Forward declarations
class DistributedType;
class Class;
class Struct;
class Field;
class HashGenerator;

struct Import {
    std::string module;
    std::vector<std::string> symbols;

    inline Import(const std::string& module_name);
};

// A File represents the complete list of Distributed Class descriptions as read from a .dc file.
class File
{

  public:
    File(); // constructor
    ~File(); // destructor

    // get_num_classes returns the number of classes in the file
    inline size_t get_num_classes() const;
    // get_class returns the <n>th class read from the .dc file(s).
    inline Class* get_class(unsigned int n);
    inline const Class* get_class(unsigned int n) const;

    // get_num_structs returns the number of structs in the file.
    inline size_t get_num_structs() const;
    // get_struct returns the <n>th struct in the file.
    inline Struct* get_struct(unsigned int n);
    inline const Struct* get_struct(unsigned int n) const;

    // get_class_by_id returns the requested class or nullptr if there is no such class.
    Class* get_class_by_id(unsigned int id);
    const Class* get_class_by_id(unsigned int id) const;
    // get_class_by_name returns the requested class or nullptr if there is no such class.
    Class* get_class_by_name(const std::string &name);
    const Class* get_class_by_name(const std::string &name) const;

    // get_num_types returns the number of types in the file.
    //     All type ids will be within the range 0 <= id < get_num_types().
    inline size_t get_num_types() const;
    // get_type_by_id returns the requested type or nullptr if there is no such type.
    inline DistributedType* get_type_by_id(unsigned int id);
    inline const DistributedType* get_type_by_id(unsigned int id) const;
    // get_type_by_name returns the requested type or nullptr if there is no such type.
    inline DistributedType* get_type_by_name(const std::string &name);
    inline const DistributedType* get_type_by_name(const std::string &name) const;

    // get_field_by_id returns the request field or nullptr if there is no such field.
    inline Field* get_field_by_id(unsigned int id);
    inline const Field* get_field_by_id(unsigned int id) const;

    // get_num_imports returns the number of imports in the file.
    inline size_t get_num_imports() const;
    // get_import retuns the <n>th import in the file.
    inline Import* get_import(unsigned int n);
    inline const Import* get_import(unsigned int n) const;

    // has_keyword returns true if a keyword with the name <keyword> is declared in the file.
    inline bool has_keyword(const std::string& keyword) const;
    // get_num_keywords returns the number of keywords declared in the file.
    inline size_t get_num_keywords() const;
    // get_keyword returns the <n>th keyword declared in the file.
    inline const std::string& get_keyword(unsigned int n) const;

    // add_class adds the newly-allocated class to the file.
    //     Returns false if there is a name conflict.
    bool add_class(Class *dclass);

    // add_struct adds the newly-allocated struct definition to the file.
    //     Returns false if there is a name conflict.
    bool add_struct(Struct *dstruct);

    // add_import adds a newly-allocated import to the file.
    //     Imports with duplicate modules are combined.
    void add_import(Import* import);

    // add_typedef adds the alias <name> to the file for the type <type>.
    //     Returns false if there is a name conflict.
    bool add_typedef(const std::string& name, DistributedType* type);

    // add_keyword adds a keyword with the name <keyword> to the list of declared keywords.
    void add_keyword(const std::string &keyword);

    // get_hash returns a 32-bit hash representing the file.
    uint32_t get_hash() const;

    // generate_hash accumulates the properties of this file into the hash.
    virtual void generate_hash(HashGenerator& hashgen) const;

  private:
    // add_field gives the field a unique id within the file.
    void add_field(Field *field);
    friend class Class;
    friend class Struct;

    std::vector<Struct*> m_structs;
    std::vector<Class*> m_classes;
    std::vector<Import*> m_imports; // list of python imports in the file
    std::vector<std::string> m_keywords;

    std::vector<Field*> m_fields_by_id;
    std::vector<DistributedType*> m_types_by_id;
    std::unordered_map<std::string, DistributedType*> m_types_by_name;
};


} // close namespace dclass
#include "File.ipp"
