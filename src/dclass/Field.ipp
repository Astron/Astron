// Filename: Field.ipp
// Created by: drose (10 Jan, 2006)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
namespace dclass   // open namespace
{


// get_id returns a unique index number associated with this field.
inline unsigned int Field::get_id() const
{
	return m_id;
}

// get_class returns the Class pointer for the class that contains this field.
inline Struct *Field::get_class() const
{
	return m_class;
}

// has_default_value returns true if a default value has been explicitly
//     established for this field, false otherwise.
inline bool Field::has_default_value() const
{
	return m_has_default_value;
}

// get_default_value returns the default value for this field.  If a
//     default value has been explicitly set (e.g. has_default_value() returns true),
//     returns that value; otherwise, returns an implicit default for the field.
inline const std::string &Field::get_default_value() const
{
	if(m_default_value_stale)
	{
		((Field*)this)->refresh_default_value();
	}
	return m_default_value;
}

// is_required returns true if the "required" flag is set for this field, false otherwise.
inline bool Field::is_required() const
{
	return has_keyword("required");
}

// is_broadcast returns true if the "broadcast" flag is set for this field, false otherwise.
inline bool Field::is_broadcast() const
{
	return has_keyword("broadcast");
}

// is_ram returns true if the "ram" flag is set for this field, false otherwise.
inline bool Field::is_ram() const
{
	return has_keyword("ram");
}

// is_db returns true if the "db" flag is set for this field, false otherwise.
inline bool Field::is_db() const
{
	return has_keyword("db");
}

// is_clsend returns true if the "clsend" flag is set for this field, false otherwise.
inline bool Field::is_clsend() const
{
	return has_keyword("clsend");
}

// is_clrecv returns true if the "clrecv" flag is set for this field, false otherwise.
inline bool Field::is_clrecv() const
{
	return has_keyword("clrecv");
}

// is_ownsend returns true if the "ownsend" flag is set for this field, false otherwise.
inline bool Field::is_ownsend() const
{
	return has_keyword("ownsend");
}

// is_ownrecv returns true if the "ownrecv" flag is set for this field, false otherwise.
inline bool Field::is_ownrecv() const
{
	return has_keyword("ownrecv");
}

// is_airecv returns true if the "airecv" flag is set for this field, false otherwise.
inline bool Field::is_airecv() const
{
	return has_keyword("airecv");
}

// output writes a string representation of this instance to <out>.
inline void Field::output(std::ostream &out) const
{
	output(out, true);
}

// write writes a string representation of this instance to <out>.
inline void Field::write(std::ostream &out, int indent_level) const
{
	write(out, false, indent_level);
}

// set_number assigns the unique number to this field.  This is normally called
//     only by the Class interface as the field is added.
inline void Field::set_id(unsigned int number)
{
	m_id = number;
}

// set_class assigns the class pointer to this field.  This is normally called
//     only by the Class interface as the field is added.
inline void Field::set_class(Struct *dclass)
{
	m_class = dclass;
}

inline void Field::set_name(const std::string& name)
{
	m_name = name;
}

// set_default_value establishes a default value for this field.
inline void Field::set_default_value(const std::string &default_value)
{
	m_default_value = default_value;
	m_has_default_value = true;
	m_default_value_stale = false;
}


} // close namespace dclass
