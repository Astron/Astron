// Filename: Field.cpp
// Created by: drose (11 Oct, 2000)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Field.h"
#include "File.h"
#include "Class.h"
#include "HashGenerator.h"
namespace dclass   // open namespace
{


// nameless constructor (for structs)
Field::Field() : m_class(NULL), m_id(0),
	m_default_value_stale(true), m_has_default_value(false)
{
	m_datatype = DT_method;
}

// named constructor (for classes)
Field::Field(const std::string &name, Struct *dclass) : m_name(name), m_class(dclass),
	m_id(0), m_default_value_stale(true), m_has_default_value(false)
{
	m_datatype = DT_method;
}

// destructor
Field::~Field()
{
}

// as_field returns the same pointer converted to a field pointer,
//     if this is in fact a field; otherwise, returns NULL.
Field* Field::as_field()
{
	return this;
}

// as_field returns the same pointer converted to a field pointer,
//     if this is in fact a field; otherwise, returns NULL.
const Field* Field::as_field() const
{
	return this;
}

// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
AtomicField* Field::as_atomic_field()
{
	return (AtomicField*)NULL;
}

// as_atomic_field returns the same field pointer converted to an atomic field
//     pointer, if this is in fact an atomic field; otherwise, returns NULL.
const AtomicField* Field::as_atomic_field() const
{
	return (AtomicField*)NULL;
}

// as_molecular_field returns the same field pointer converted to a molecular field
//     pointer, if this is in fact a molecular field; otherwise, returns NULL.
MolecularField* Field::as_molecular_field()
{
	return (MolecularField*)NULL;
}

// as_molecular_field returns the same field pointer converted to a molecular field
//     pointer, if this is in fact a molecular field; otherwise, returns NULL.
const MolecularField* Field::as_molecular_field() const
{
	return (MolecularField*)NULL;
}

// as_parameter returns the same field pointer converted to a parameter
//     pointer, if this is in fact a parameter; otherwise, returns NULL.
Parameter* Field::as_parameter()
{
	return (Parameter*)NULL;
}

// as_parameter returns the same field pointer converted to a parameter
//     pointer, if this is in fact a parameter; otherwise, returns NULL.
const Parameter* Field::as_parameter() const
{
	return (Parameter*)NULL;
}

// generate_hash accumulates the properties of this field into the hash.
void Field::generate_hash(HashGenerator &hashgen) const
{
	// It shouldn't be necessary to explicitly add _number to the
	// hash--this is computed based on the relative position of this
	// field with the other fields, so adding it explicitly will be
	// redundant.  However, the field name is significant.
	hashgen.add_string(m_name);

	// Actually, we add _number anyway, since we need to ensure the hash
	// code comes out different in the dc_multiple_inheritance case.
	hashgen.add_int(m_id);
}

void Field::refresh_default_value()
{
	// TODO: this
}


} // close namespace dclass
