// Filename: MolecularField.h
#pragma once
#include "Field.h"
#include "StrucType.h"
namespace dclass   // open namespace dclass
{


// A MolecularField is an abstract field which provides an interface that can
//     be used to access multiple other fields at the same time.
class MolecularField : public Field, public StructType
{
	public:
		MolecularField(Class* cls, const std::string &name);

		// as_molecular returns this as a MolecularField if it is molecular, or NULL otherwise.
		virtual MolecularField* as_molecular();
		virtual const MolecularField* as_molecular() const;

		// Use the field interface by default and not the struct interface
		using Field::get_id;
		using Field::get_name;
		using Field::get_type;
		using Field::set_id;
		using Field::set_name;

		// add_field adds a new Field as part of the Molecular.
		//     Returns false if the field could not be added.
		virtual bool add_field(Field *field);

		// generate_hash accumulates the properties of this field into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;
};


} // close namespace dclass
