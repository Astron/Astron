// Filename: Switch.cpp
// Created by: drose (23 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Switch.h"
#include "Field.h"
#include "Parameter.h"
#include "HashGenerator.h"
#include "indent.h"
#include "Packer.h"
namespace dclass   // open namespace dclass
{


// constructor -- key_parameter must be recently allocated via new;
//     it will be deleted via delete when the switch destructs.
Switch::Switch(const string &name, Field *key_parameter) :
	m_name(name), m_key_parameter(key_parameter), m_default_case(NULL), m_fields_added(false)
{
}

// destructor
Switch::~Switch()
{
	nassertv(m_key_parameter != (Field *)NULL);
	delete m_key_parameter;

	for(auto it = m_cases.begin(); it != m_cases.end(); ++it)
	{
		SwitchCase *dcase = (*it);
		delete dcase;
	}

	for(auto it = m_case_fields.begin(); it != m_case_fields.end(); ++it)
	{
		SwitchFields *fields = (*it);
		delete fields;
	}

	for(auto it = m_nested_fields.begin(); it != m_nested_fields.end(); ++it)
	{
		Field *field = (*it);
		delete field;
	}
}

// as_switch returns the same declaration pointer converted to a switch,
//     if this is in fact a switch; otherwise, returns NULL.
Switch *Switch::as_switch()
{
	return this;
}

// as_switch returns the same declaration pointer converted to a switch,
//     if this is in fact a switch; otherwise, returns NULL.
const Switch *Switch::as_switch() const
{
	return this;
}

// get_name returns the name of this switch.
const std::string &Switch::get_name() const
{
	return m_name;
}

// get_key_parameter returns the key parameter on which the switch is based.
//     The value of this parameter in the record determines which one of the
//     several cases within the switch will be used.
Field *Switch::get_key_parameter() const
{
	return m_key_parameter;
}

// get_num_cases returns the number of different cases within the switch.
//     The legal values for case_index range from 0 to get_num_cases() - 1.
int Switch::get_num_cases() const
{
	return m_cases.size();
}

// get_case_by_value returns the index number of the case with the indicated packed value,
//     or -1 if no case has this value.
int Switch::get_case_by_value(const std::string &case_value) const
{
	auto case_it = m_cases_by_value.find(case_value);
	if(case_it != m_cases_by_value.end())
	{
		return case_it->second;
	}

	return -1;
}

// get_case returns the PackerInterface that packs the nth case.
PackerInterface *Switch::get_case(int n) const
{
	nassertr(n >= 0 && n < (int)m_cases.size(), NULL);
	return m_cases[n]->m_fields;
}

// get_default_case returns the PackerInterface that packs the default case,
//     or NULL if there is no default case.
PackerInterface *Switch::get_default_case() const
{
	return m_default_case;
}

// get_value returns the packed value associated with the indicated case.
std::string Switch::get_value(int case_index) const
{
	nassertr(case_index >= 0 && case_index < (int)m_cases.size(), string());
	return m_cases[case_index]->m_value;
}

// get_num_fields returns the number of fields in the indicated case.
int Switch::get_num_fields(int case_index) const
{
	nassertr(case_index >= 0 && case_index < (int)m_cases.size(), 0);
	return m_cases[case_index]->m_fields->m_fields.size();
}

// get_num_fields returns the nth field in the indicated case.
Field *Switch::get_field(int case_index, int n) const
{
	nassertr(case_index >= 0 && case_index < (int)m_cases.size(), NULL);
	nassertr(n >= 0 && n < (int)m_cases[case_index]->m_fields->m_fields.size(), NULL);
	return m_cases[case_index]->m_fields->m_fields[n];
}

// get_field_by_name returns the field with the given name from the indicated case,
//     or NULL if no field has this name.
Field *Switch::get_field_by_name(int case_index, const string &name) const
{
	nassertr(case_index >= 0 && case_index < (int)m_cases.size(), NULL);

	const std::map<std::string, Field*> &fields_by_name = m_cases[case_index]->m_fields->m_fields_by_name;
	auto field_it = fields_by_name.find(name);
	if(field_it != fields_by_name.end())
	{
		return field_it->second;
	}

	return NULL;
}

// is_field_valid returns true if it is valid to add a new field at this point
//     (implying that a case or default has been added already), or false if not.
bool Switch::is_field_valid() const
{
	return !m_current_fields.empty();
}

// add_case adds a new case to the switch with the indicated value, and returns the new case_index.
//     If the value has already been used for another case, returns -1.
int Switch::add_case(const string &value)
{
	int case_index = (int)m_cases.size();
	if(!m_cases_by_value.insert(std::map<std::string, int>::value_type(value, case_index)).second)
	{
		add_invalid_case();
		return -1;
	}

	SwitchFields *fields = start_new_case();
	SwitchCase *dcase = new SwitchCase(value, fields);
	m_cases.push_back(dcase);
	return case_index;
}

// add_invalid_case adds a new case to the switch that will never be matched.
//     This is only used by the parser, to handle an error condition more gracefully
//     without bitching the parsing (which behaves differently according to whether
//     a case has been encountered or not).
void Switch::add_invalid_case()
{
	start_new_case();
}

// add_default adds a default case to the switch.  Returns true if the case is successfully added,
//     or false if it had already been added.  This is normally called only by the parser.
bool Switch::add_default()
{
	if(m_default_case != (SwitchFields *)NULL)
	{
		add_invalid_case();
		return false;
	}

	SwitchFields *fields = start_new_case();
	m_default_case = fields;
	return true;
}

// add_field adds a field to the currently active cases (those that have been added via add_case()
//     or add_default(), since the last call to add_break()).  Returns true if successful,
//     false if the field duplicates a field already named within this case.
//     It is an error to call this before calling add_case() or add_default().
bool Switch::add_field(Field *field)
{
	nassertr(!m_current_fields.empty(), false);

	bool all_ok = true;

	for(auto it = m_current_fields.begin(); it != m_current_fields.end(); ++it)
	{
		SwitchFields *fields = (*it);
		if(!fields->add_field(field))
		{
			all_ok = false;
		}
	}
	m_nested_fields.push_back(field);
	m_fields_added = true;

	return all_ok;
}

// add_break adds a break statement to the switch.
//     This closes the currently open cases and prepares for a new, unrelated case.
void Switch::add_break()
{
	m_current_fields.clear();
	m_fields_added = false;
}

// apply_switch returns the PackerInterface that presents the alternative fields for the case
//     indicated by the given packed value string, or NULL if the value string does not match
//     one of the expected cases.
const PackerInterface *Switch::apply_switch(const char *value_data, size_t length) const
{
	auto case_it = m_cases_by_value.find(string(value_data, length));
	if(case_it != m_cases_by_value.end())
	{
		return m_cases[case_it->second]->m_fields;
	}

	// Unexpected value--use the default.
	if(m_default_case != (SwitchFields *)NULL)
	{
		return m_default_case;
	}

	// No default.
	return NULL;
}

// output writes a string representation of this instance to <out>.
void Switch::output(std::ostream &out, bool brief) const
{
	output_instance(out, brief, "", "", "");
}

// write generates a parseable description of the object to the indicated output stream.
void Switch::write(std::ostream &out, bool brief, int indent_level) const
{
	write_instance(out, brief, indent_level, "", "", "");
}

// output_instance generates a parseable description of the object to the indicated output stream.
void Switch::output_instance(std::ostream &out, bool brief, const std::string &prename,
                             const std::string &name, const std::string &postname) const
{
	out << "switch";
	if(!m_name.empty())
	{
		out << " " << m_name;
	}
	out << " (";
	m_key_parameter->output(out, brief);
	out << ") {";

	const SwitchFields *last_fields = NULL;

	for(auto it = m_cases.begin(); it != m_cases.end(); ++it)
	{
		const SwitchCase *dcase = (*it);
		if(dcase->m_fields != last_fields && last_fields != (SwitchFields *)NULL)
		{
			last_fields->output(out, brief);
		}
		last_fields = dcase->m_fields;
		out << "case " << m_key_parameter->format_data(dcase->m_value, false) << ": ";
	}

	if(m_default_case != (SwitchFields *)NULL)
	{
		if(m_default_case != last_fields && last_fields != (SwitchFields *)NULL)
		{
			last_fields->output(out, brief);
		}
		last_fields = m_default_case;
		out << "default: ";
	}
	if(last_fields != (SwitchFields *)NULL)
	{
		last_fields->output(out, brief);
	}

	out << "}";
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
}

// write_instance generates a parseable description of the object to the indicated output stream.
void Switch::write_instance(std::ostream &out, bool brief, int indent_level,
                            const std::string &prename, const std::string &name,
                            const std::string &postname) const
{
	indent(out, indent_level) << "switch";
	if(!m_name.empty())
	{
		out << " " << m_name;
	}
	out << " (";
	m_key_parameter->output(out, brief);
	out << ") {\n";

	const SwitchFields *last_fields = NULL;

	for(auto it = m_cases.begin(); it != m_cases.end(); ++it)
	{
		const SwitchCase *dcase = (*it);
		if(dcase->m_fields != last_fields && last_fields != (SwitchFields *)NULL)
		{
			last_fields->write(out, brief, indent_level + 2);
		}
		last_fields = dcase->m_fields;
		indent(out, indent_level) << "case "
			<< m_key_parameter->format_data(dcase->m_value, false) << ":\n";
	}

	if(m_default_case != (SwitchFields *)NULL)
	{
		if(m_default_case != last_fields && last_fields != (SwitchFields *)NULL)
		{
			last_fields->write(out, brief, indent_level + 2);
		}
		last_fields = m_default_case;
		indent(out, indent_level) << "default:\n";
	}
	if(last_fields != (SwitchFields *)NULL)
	{
		last_fields->write(out, brief, indent_level + 2);
	}

	indent(out, indent_level) << "}";
	if(!prename.empty() || !name.empty() || !postname.empty())
	{
		out << " " << prename << name << postname;
	}
	out << ";\n";
}

// generate_hash accumulates the properties of this switch into the hash.
void Switch::generate_hash(HashGenerator &hashgen) const
{
	hashgen.add_string(m_name);

	m_key_parameter->generate_hash(hashgen);

	hashgen.add_int(m_cases.size());
	for(auto case_it = m_cases.begin(); case_it != m_cases.end(); ++case_it)
	{
		const SwitchCase *dcase = (*case_it);
		hashgen.add_string(dcase->m_value);

		const SwitchFields *fields = dcase->m_fields;
		hashgen.add_int(fields->m_fields.size());
		for(auto field_it = fields->m_fields.begin(); field_it != fields->m_fields.end(); ++field_it)
		{
			(*field_it)->generate_hash(hashgen);
		}
	}

	if(m_default_case != (SwitchFields *)NULL)
	{
		const SwitchFields *fields = m_default_case;
		hashgen.add_int(fields->m_fields.size());
		for(auto field_it = fields->m_fields.begin(); field_it != fields->m_fields.end(); ++field_it)
		{
			(*field_it)->generate_hash(hashgen);
		}
	}
}

// pack_default_value packs the switchParameter's specified default value
//     (or a sensible default if no value is specified) into the stream.
//     Returns true if the default value is packed, false if the switchParameter
//     doesn't know how to pack its default value.
bool Switch::pack_default_value(PackData &pack_data, bool &pack_error) const
{
	SwitchFields *fields = NULL;
	Packer packer;
	packer.begin_pack(m_key_parameter);
	if(!m_cases.empty())
	{
		// If we have any cases, the first case is always the default
		// case, regardless of the default value specified by the key
		// parameter.  That's just the easiest to code.
		packer.pack_literal_value(m_cases[0]->m_value);
		fields = m_cases[0]->m_fields;

	}
	else
	{
		// If we don't have any cases, just pack the key parameter's
		// default.
		packer.pack_default_value();
		fields = m_default_case;
	}

	if(!packer.end_pack())
	{
		pack_error = true;
	}

	if(fields == (SwitchFields *)NULL)
	{
		pack_error = true;

	}
	else
	{
		// Then everything within the case gets its normal default.
		for(size_t i = 1; i < fields->m_fields.size(); i++)
		{
			packer.begin_pack(fields->m_fields[i]);
			packer.pack_default_value();
			if(!packer.end_pack())
			{
				pack_error = true;
			}
		}
	}

	pack_data.append_data(packer.get_data(), packer.get_length());

	return true;
}

// do_check_match_switch returns true if this switch matches the indicated other switch
//     that is, the two switches are bitwise equivalent--false otherwise.
//     This is only intended to be called internally from do_check_match_switch_parameter().
bool Switch::do_check_match_switch(const Switch *other) const
{
	if(!m_key_parameter->check_match(other->m_key_parameter))
	{
		return false;
	}

	if(m_cases.size() != other->m_cases.size())
	{
		return false;
	}

	for(auto case_it = m_cases.begin(); case_it != m_cases.end(); ++case_it)
	{
		const SwitchCase *c1 = (*case_it);
		auto val_it = other->m_cases_by_value.find(c1->m_value);
		if(val_it == other->m_cases_by_value.end())
		{
			// No matching value.
			return false;
		}
		int c2_index = val_it->second;
		nassertr(c2_index >= 0 && c2_index < (int)other->m_cases.size(), false);
		const SwitchCase *c2 = other->m_cases[c2_index];

		if(!c1->do_check_match_switch_case(c2))
		{
			return false;
		}
	}

	return true;
}

// start_new_case creates a new field set for the new case, or shares the field set with the
//     previous case, as appropriate. Returns the appropriate field set.
Switch::SwitchFields *Switch::start_new_case()
{
	SwitchFields *fields = NULL;

	if(m_current_fields.empty() || m_fields_added)
	{
		// If we have recently encountered a break (which removes all of
		// the current field sets) or if we have already added at least
		// one field to the previous case without an intervening break,
		// then we can't share the field set with the previous case.
		// Create a new one.
		fields = new SwitchFields(m_name);
		fields->add_field(m_key_parameter);

		m_case_fields.push_back(fields);
		m_current_fields.push_back(fields);

	}
	else
	{
		// Otherwise, we can share the field set with the previous case.
		fields = m_current_fields.back();
	}

	m_fields_added = false;

	return fields;
}


// constructor
Switch::SwitchFields::SwitchFields(const string &name) : PackerInterface(name)
{
	m_has_nested_fields = true;
	m_num_nested_fields = 0;
	m_pack_type = PT_switch;

	m_has_fixed_byte_size = true;
	m_fixed_byte_size = 0;
	m_has_fixed_structure = true;
	m_has_range_limits = false;
	m_has_default_value = false;
}

// destructor
Switch::SwitchFields::~SwitchFields()
{
	// We don't delete any of the nested fields here, since they might
	// be shared by multiple SwitchFields objects.  Instead, we delete
	// them in the Switch destructor.
}

// get_nested_field returns the PackerInterface object that represents the nth nested field.
//     This may return NULL if n is out of the range 0 <= n < get_num_nested_fields()).
PackerInterface *Switch::SwitchFields::get_nested_field(int n) const
{
	nassertr(n >= 0 && n < (int)m_fields.size(), NULL);
	return m_fields[n];
}

// add_field adds a field to this case.  Returns true if successful, or false if the
//     field duplicates a field already named within this case.
bool Switch::SwitchFields::add_field(Field *field)
{
	if(!field->get_name().empty())
	{
		bool inserted = m_fields_by_name.insert(
			std::map<std::string, Field*>::value_type(field->get_name(), field)).second;

		if(!inserted)
		{
			return false;
		}
	}

	m_fields.push_back(field);

	m_num_nested_fields = (int)m_fields.size();

	// See if we still have a fixed byte size.
	if(m_has_fixed_byte_size)
	{
		m_has_fixed_byte_size = field->has_fixed_byte_size();
		m_fixed_byte_size += field->get_fixed_byte_size();
	}
	if(m_has_fixed_structure)
	{
		m_has_fixed_structure = field->has_fixed_structure();
	}
	if(!m_has_range_limits)
	{
		m_has_range_limits = field->has_range_limits();
	}
	if(!m_has_default_value)
	{
		m_has_default_value = field->has_default_value();
	}
	return true;
}

// do_check_match_switch_case returns true if this case matches the indicated case, false otherwise.
//     This is only intended to be called internally from Switch::do_check_match_switch().
bool Switch::SwitchFields::do_check_match_switch_case(const Switch::SwitchFields *other) const
{
	if(m_fields.size() != other->m_fields.size())
	{
		return false;
	}
	for(size_t i = 0; i < m_fields.size(); i++)
	{
		if(!m_fields[i]->check_match(other->m_fields[i]))
		{
			return false;
		}
	}

	return true;
}

// output writes a string representation of the SwitchFields to <out>.
void Switch::SwitchFields::output(std::ostream &out, bool brief) const
{
	if(!m_fields.empty())
	{
		auto field_it = m_fields.begin();
		++field_it;
		while(field_it != m_fields.end())
		{
			(*field_it)->output(out, brief);
			out << "; ";
			++field_it;
		}
	}
	out << "break; ";
}

// write writes a string representation of the SwitchFields to <out>.
void Switch::SwitchFields::write(std::ostream &out, bool brief, int indent_level) const
{
	if(!m_fields.empty())
	{
		auto field_it = m_fields.begin();
		++field_it;
		while(field_it != m_fields.end())
		{
			(*field_it)->write(out, brief, indent_level);
			++field_it;
		}
	}
	indent(out, indent_level) << "break;\n";
}

// do_check_match returns true if the other interface is bitwise the same as this one
//     that is, a uint32 only matches a uint32, etc. are not compared.
bool Switch::SwitchFields::do_check_match(const PackerInterface *) const
{
	// This should never be called on a SwitchFields.
	nassertr(false, false);
	return false;
}

// constructor
Switch::SwitchCase::SwitchCase(const std::string &value, Switch::SwitchFields *fields) :
	m_value(value), m_fields(fields)
{
}

// destructor
Switch::SwitchCase::~SwitchCase()
{
}

// do_check_match_switch_case returns true if this case matches the indicated case, false otherwise.
bool Switch::SwitchCase::do_check_match_switch_case(const Switch::SwitchCase *other) const
{
	return m_fields->do_check_match_switch_case(other->m_fields);
}


} // close namespace dclass
