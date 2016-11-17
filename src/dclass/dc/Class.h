// Filename: Class.h
#pragma once
#include "Struct.h"
#include <unordered_map> // std::unordered_map
namespace dclass   // open namespace
{


// A Class is a special type of struct that have a couple advanced object-oriented features:
//     Classes can inherit from other classes (ie. have super-/sub-classes).
//     Classes can have methods including a special constructor method.
//     Unlike other structs, classes cannot have anonymous fields.
class Class : public Struct
{
  public:
    Class(File *dc_file, const std::string &name);
    virtual ~Class();

    // as_class returns this Struct as a Class if it is a Class, or nullptr otherwise.
    virtual Class* as_class();
    virtual const Class* as_class() const;

    // get_num_parents returns the number of superclasses this class inherits from.
    inline size_t get_num_parents() const;
    // get_parent returns the <n>th parent-/super-class this class inherits from.
    inline Class* get_parent(unsigned int n);
    inline const Class* get_parent(unsigned int n) const;
    // get_num_children returns the number of subclasses that inherit from this class.
    inline size_t get_num_children() const;
    // get_child returns the <n>th child-/sub-class that inherits this class.
    inline Class* get_child(unsigned int n);
    inline const Class* get_child(unsigned int n) const;

    // has_constructor returns true if this class has a constructor method,
    //     or false if it just uses the default constructor.
    inline bool has_constructor() const;
    // get_constructor returns the constructor method for this class if it is defined,
    //     or nullptr if the class uses the default constructor.
    inline Field* get_constructor();
    inline const Field* get_constructor() const;

    // get_num_base_fields returns the number of fields declared directly in this class.
    inline size_t get_num_base_fields() const;
    // get_base_field returns the <n>th field from the class excluding any inherited fields.
    inline Field* get_base_field(unsigned int n);
    inline const Field* get_base_field(unsigned int n) const;

    // add_parent set this class as a subclass to target parent.
    void add_parent(Class *parent);

    // add_field adds a new Field to the class.
    virtual bool add_field(Field* field);

    // generate_hash accumulates the properties of this type into the hash.
    virtual void generate_hash(HashGenerator &hashgen) const;

  private:
    // add_child marks a class as a child of this class.
    void add_child(Class* child);
    // add_inherited_field updates a classes's fields after a parent adds a new field.
    void add_inherited_field(Class* parent, Field* field);
    // shadow_field removes the field from all of the Class's field accessors,
    //     so that another field with the same name can be inserted.
    void shadow_field(Field* field);

    Field* m_constructor;
    std::vector<Field*> m_base_fields;
    std::unordered_map<std::string, Field*> m_base_fields_by_name;

    std::vector<Class*> m_parents;
    std::vector<Class*> m_children;
};


} // close namespace dclass
#include "Class.ipp"
