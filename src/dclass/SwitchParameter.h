// Filename: ClassParameter.h
// Created by: drose (29 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "Parameter.h"
namespace dclass   // open namespace dclass
{


// Foward declarations
class Switch;

// A SwitchParameter represents a switch object used as a parameter itself,
//     which packs the appropriate fields of the switch into the message.
class EXPCL_DIRECT SwitchParameter : public Parameter
{
	public:
		SwitchParameter(const Switch *dswitch);
		SwitchParameter(const SwitchParameter &copy);

	PUBLISHED:
		virtual SwitchParameter *as_switch_parameter();
		virtual const SwitchParameter *as_switch_parameter() const;
		virtual Parameter *make_copy() const;
		virtual bool is_valid() const;

		const Switch *get_switch() const;

	public:
		virtual PackerInterface *get_nested_field(int n) const;

		const PackerInterface *apply_switch(const char *value_data, size_t length) const;

		virtual void output_instance(ostream &out, bool brief, const std::string &prename,
		                             const string &name, const std::string &postname) const;
		virtual void write_instance(std::ostream &out, bool brief, int indent_level,
		                            const std::string &prename, const std::string &name,
		                            const std::string &postname) const;
		virtual void generate_hash(HashGenerator &hashgen) const;
		virtual bool pack_default_value(PackData &pack_data, bool &pack_error) const;

	protected:
		virtual bool do_check_match(const PackerInterface *other) const;
		virtual bool do_check_match_switch_parameter(const SwitchParameter *other) const;

	private:
		const Switch *_dswitch;
};


} // close namespace dclass
