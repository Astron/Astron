// Filename: MolecularField.cpp
#include "util/HashGenerator.h"
#include "dc/Class.h"

#include "MolecularField.h"
namespace dclass   // open namespace dclass
{


// constructor
MolecularField::MolecularField(Class* cls, const std::string &name) :
    Field(nullptr, name), Struct(cls->get_file())
{
    Field::m_type = this;
}

// destructor
MolecularField::~MolecularField()
{
}

// as_molecular returns this as a MolecularField if it is molecular, or nullptr otherwise.
MolecularField* MolecularField::as_molecular()
{
    return this;
}
const MolecularField* MolecularField::as_molecular() const
{
    return this;
}

// add_field adds a new Field as part of the Molecular.
//     Returns false if the field could not be added.
bool MolecularField::add_field(Field* field)
{
    // Moleculars cannot be nested
    if(field->as_molecular()) {
        return false;
    }

    // Copy our keywords from the first add field
    if(m_fields.size() == 0) {
        copy_keywords(*field);
    }

    // Each field has to have the same set of keywords
    else if(!has_matching_keywords(*field)) {
        return false;
    }

    // Add the field
    m_fields.push_back(field);

    // See if we still have a fixed byte size.
    if(has_fixed_size() || m_fields.size() == 1) {
        if(field->get_type()->has_fixed_size()) {
            m_size += field->get_type()->get_size();
        } else {
            m_size = 0;
        }
    }

    if(has_default_value() || m_fields.size() == 1) {
        // If any atomic field of the molecular has a default value,
        // the molecular is also considerd to have a default value.
        if(field->has_default_value()) {
            m_default_value = true;
        }

        m_default_value += field->get_default_value();
    }

    return true;
}

bool MolecularField::set_default_value(const std::string&)
{
    // MolecularField default values are implict from their
    // atomic components and cannot be defined manually.
    return false;
}

// generate_hash accumulates the properties of this field into the hash.
void MolecularField::generate_hash(HashGenerator& hashgen) const
{
    hashgen.add_int(Field::m_id);
    hashgen.add_string(Field::m_name);
    // We aren't the owner of the fields so we only use their id in the hash
    hashgen.add_int(m_fields.size());
    for(const auto& it : m_fields) {
        hashgen.add_int(it->get_id());
    }
}


} // close namespace dclass
