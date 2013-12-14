// Filename: PackerInterface.cxx
// Created by: drose (15 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "PackerInterface.h"
#include "PackerCatalog.h"
#include "Field.h"
#include "ParserDefs.h"
#include "LexerDefs.h"
namespace dclass   // open namespace dclass
{


////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PackerInterface::
PackerInterface(const string &name) :
	_name(name)
{
	_has_fixed_byte_size = false;
	_fixed_byte_size = 0;
	_has_fixed_structure = false;
	_has_range_limits = false;
	_num_length_bytes = 0;
	_has_nested_fields = false;
	_num_nested_fields = -1;
	_pack_type = PT_invalid;
	_catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PackerInterface::
PackerInterface(const PackerInterface &copy) :
	_name(copy._name),
	_has_fixed_byte_size(copy._has_fixed_byte_size),
	_fixed_byte_size(copy._fixed_byte_size),
	_has_fixed_structure(copy._has_fixed_structure),
	_has_range_limits(copy._has_range_limits),
	_num_length_bytes(copy._num_length_bytes),
	_has_nested_fields(copy._has_nested_fields),
	_num_nested_fields(copy._num_nested_fields),
	_pack_type(copy._pack_type)
{
	_catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PackerInterface::
~PackerInterface()
{
	if(_catalog != (PackerCatalog *)NULL)
	{
		delete _catalog;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::find_seek_index
//       Access: Published
//  Description: Returns the index number to be passed to a future
//               call to Packer::seek() to seek directly to the
//               named field without having to look up the field name
//               in a table later, or -1 if the named field cannot be
//               found.
//
//               If the named field is nested within a switch or some
//               similar dynamic structure that reveals different
//               fields based on the contents of the data, this
//               mechanism cannot be used to pre-fetch the field index
//               number--you must seek for the field by name.
////////////////////////////////////////////////////////////////////
int PackerInterface::
find_seek_index(const string &name) const
{
	return get_catalog()->find_entry_by_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_field
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Field *PackerInterface::
as_field()
{
	return (Field *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_field
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
const Field *PackerInterface::
as_field() const
{
	return (Field *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_switch_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
SwitchParameter *PackerInterface::
as_switch_parameter()
{
	return (SwitchParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_switch_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
const SwitchParameter *PackerInterface::
as_switch_parameter() const
{
	return (SwitchParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_class_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ClassParameter *PackerInterface::
as_class_parameter()
{
	return (ClassParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::as_class_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
const ClassParameter *PackerInterface::
as_class_parameter() const
{
	return (ClassParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::check_match
//       Access: Published
//  Description: Returns true if this interface is bitwise the same as
//               the interface described with the indicated formatted
//               string, e.g. "(uint8, uint8, int16)", or false
//               otherwise.
//
//               If File is not NULL, it specifies the File that
//               was previously loaded, from which some predefined
//               structs and typedefs may be referenced in the
//               description string.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
check_match(const string &description, File *dcfile) const
{
	bool match = false;

	istringstream strm(description);
	dc_init_parser_parameter_description(strm, "check_match", dcfile);
	dcyyparse();
	dc_cleanup_parser();

	Field *field = dc_get_parameter_description();
	if(field != NULL)
	{
		match = check_match(field);
		delete field;
	}

	if(dc_error_count() == 0)
	{
		return match;
	}

	// Parse error: no match is allowed.
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::set_name
//       Access: Public, Virtual
//  Description: Sets the name of this field.
////////////////////////////////////////////////////////////////////
void PackerInterface::
set_name(const string &name)
{
	_name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::calc_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the _num_length_bytes stored in the stream on the
//               push).  This will only be called if _num_length_bytes
//               is nonzero.
////////////////////////////////////////////////////////////////////
int PackerInterface::
calc_num_nested_fields(size_t) const
{
	return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the PackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
PackerInterface *PackerInterface::
get_nested_field(int) const
{
	return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::validate_num_nested_fields
//       Access: Public, Virtual
//  Description: After a number of fields have been packed via push()
//               .. pack_*() .. pop(), this is called to confirm that
//               the number of nested fields that were added is valid
//               for this type.  This is primarily useful for array
//               types with dynamic ranges that can't validate the
//               number of fields any other way.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
validate_num_nested_fields(int) const
{
	return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_double
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_double(PackData &, double, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_int
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_int(PackData &, int, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_uint
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_uint(PackData &, unsigned int, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_int64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_int64(PackData &, int64_t, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_uint64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_uint64(PackData &, uint64_t, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_string
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
pack_string(PackData &, const string &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::pack_default_value
//       Access: Public, Virtual
//  Description: Packs the field's specified default value (or a
//               sensible default if no value is specified) into the
//               stream.  Returns true if the default value is packed,
//               false if the field doesn't know how to pack its
//               default value.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
pack_default_value(PackData &, bool &) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_double
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_double(const char *, size_t, size_t &, double &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_int
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_int(const char *, size_t, size_t &, int &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_uint
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_uint(const char *, size_t, size_t &, unsigned int &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_int64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_int64(const char *, size_t, size_t &, int64_t &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_uint64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_uint64(const char *, size_t, size_t &, uint64_t &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.
////////////////////////////////////////////////////////////////////
void PackerInterface::
unpack_string(const char *, size_t, size_t &, string &, bool &pack_error, bool &) const
{
	pack_error = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_validate
//       Access: Public, Virtual
//  Description: Internally unpacks the current numeric or string
//               value and validates it against the type range limits,
//               but does not return the value.  Returns true on
//               success, false on failure (e.g. we don't know how to
//               validate this field).
////////////////////////////////////////////////////////////////////
bool PackerInterface::
unpack_validate(const char *data, size_t length, size_t &p,
                bool &pack_error, bool &) const
{
	if(!_has_range_limits)
	{
		return unpack_skip(data, length, p, pack_error);
	}
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::unpack_skip
//       Access: Public, Virtual
//  Description: Increments p to the end of the current field without
//               actually unpacking any data or performing any range
//               validation.  Returns true on success, false on
//               failure (e.g. we don't know how to skip this field).
////////////////////////////////////////////////////////////////////
bool PackerInterface::
unpack_skip(const char *data, size_t length, size_t &p,
            bool &pack_error) const
{
	if(_has_fixed_byte_size)
	{
		// If this field has a fixed byte size, it's easy to skip.
		p += _fixed_byte_size;
		if(p > length)
		{
			pack_error = true;
		}
		return true;
	}

	if(_has_nested_fields && _num_length_bytes != 0)
	{
		// If we have a length prefix, use that for skipping.
		if(p + _num_length_bytes > length)
		{
			pack_error = true;

		}
		else
		{
			if(_num_length_bytes == 4)
			{
				size_t this_length = do_unpack_uint32(data + p);
				p += this_length + 4;
			}
			else
			{
				size_t this_length = do_unpack_uint16(data + p);
				p += this_length + 2;
			}
			if(p > length)
			{
				pack_error = true;
			}
		}
		return true;
	}

	// Otherwise, we don't know how to skip this field (presumably it
	// can be skipped by skipping over its nested fields individually).
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::get_catalog
//       Access: Public
//  Description: Returns the PackerCatalog associated with this
//               field, listing all of the nested fields by name.
////////////////////////////////////////////////////////////////////
const PackerCatalog *PackerInterface::
get_catalog() const
{
	if(_catalog == (PackerCatalog *)NULL)
	{
		((PackerInterface *)this)->make_catalog();
	}
	return _catalog;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_simple_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               simple parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_simple_parameter(const SimpleParameter *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_class_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               class parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_class_parameter(const ClassParameter *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_switch_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               switch parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_switch_parameter(const SwitchParameter *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_array_parameter
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               array parameter, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_array_parameter(const ArrayParameter *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_atomic_field
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               atomic field, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_atomic_field(const AtomicField *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::do_check_match_molecular_field
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               molecular field, false otherwise.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
do_check_match_molecular_field(const MolecularField *) const
{
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::make_catalog
//       Access: Private
//  Description: Called internally to create a new PackerCatalog
//               object.
////////////////////////////////////////////////////////////////////
void PackerInterface::
make_catalog()
{
	nassertv(_catalog == (PackerCatalog *)NULL);
	_catalog = new PackerCatalog(this);

	_catalog->r_fill_catalog("", this, NULL, 0);
}


} // close namespace dclass
