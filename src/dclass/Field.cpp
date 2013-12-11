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
#include "Packer.h"
#include "Class.h"
#include "HashGenerator.h"
#include "msgtypes.h"
namespace dclass   // open namespace
{


// nameless constructor (for structs)
Field::Field() : m_class(NULL), m_number(-1), m_default_value_stale(true),
	m_has_default_value(false), m_bogus_field(false), m_has_nested_fields(true),
	m_num_nested_fields(0), m_pack_type(PT_field), m_has_fixed_byte_size(true),
	m_fixed_byte_size(0), m_has_fixed_structure(true)
{
}

// named constructor (for classes)
Field::Field(const string &name, Class *dclass) : PackerInterface(name),
	m_class(dclass), m_number(-1), m_default_value_stale(true), m_has_default_value(false),
	m_bogus_field(false), m_has_nested_fields(true), m_num_nested_fields(0),
	m_pack_type(PT_field), m_has_fixed_byte_size(true), m_fixed_byte_size(0),
	m_has_fixed_structure(true)
{
}

// destructor
Field::~Field()
{
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_field
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Field *Field::
as_field()
{
	return this;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_field
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
const Field *Field::
as_field() const
{
	return this;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_atomic_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
AtomicField *Field::
as_atomic_field()
{
	return (AtomicField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_atomic_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
const AtomicField *Field::
as_atomic_field() const
{
	return (AtomicField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_molecular_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
MolecularField *Field::
as_molecular_field()
{
	return (MolecularField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_molecular_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
const MolecularField *Field::
as_molecular_field() const
{
	return (MolecularField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Parameter *Field::
as_parameter()
{
	return (Parameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::as_parameter
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
const Parameter *Field::
as_parameter() const
{
	return (Parameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::format_data
//       Access: Published
//  Description: Given a blob that represents the packed data for this
//               field, returns a string formatting it for human
//               consumption.  Returns empty string if there is an error.
////////////////////////////////////////////////////////////////////
string Field::
format_data(const string &packed_data, bool show_field_names)
{
	Packer packer;
	packer.set_unpack_data(packed_data);
	packer.begin_unpack(this);
	string result = packer.unpack_and_format(show_field_names);
	if(!packer.end_unpack())
	{
		return string();
	}
	return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::parse_string
//       Access: Published
//  Description: Given a human-formatted string (for instance, as
//               returned by format_data(), above) that represents the
//               value of this field, parse the string and return the
//               corresponding packed data.  Returns empty string if
//               there is an error.
////////////////////////////////////////////////////////////////////
string Field::
parse_string(const string &formatted_string)
{
	Packer packer;
	packer.begin_pack(this);
	if(!packer.parse_and_pack(formatted_string))
	{
		// Parse error.
		return string();
	}
	if(!packer.end_pack())
	{
		// Data type mismatch.
		return string();
	}

	return packer.get_string();
}

////////////////////////////////////////////////////////////////////
//     Function: Field::validate_ranges
//       Access: Published
//  Description: Verifies that all of the packed values in the field
//               data are within the specified ranges and that there
//               are no extra bytes on the end of the record.  Returns
//               true if all fields are valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool Field::
validate_ranges(const string &packed_data) const
{
	Packer packer;
	packer.set_unpack_data(packed_data);
	packer.begin_unpack(this);
	packer.unpack_validate();
	if(!packer.end_unpack())
	{
		return false;
	}

	return (packer.get_num_unpacked_bytes() == packed_data.length());
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::pack_args
//       Access: Published
//  Description: Packs the Python arguments from the indicated tuple
//               into the packer.  Returns true on success, false on
//               failure.
//
//               It is assumed that the packer is currently positioned
//               on this field.
////////////////////////////////////////////////////////////////////
bool Field::
pack_args(Packer &packer, PyObject *sequence) const
{
	nassertr(!packer.had_error(), false);
	nassertr(packer.get_current_field() == this, false);

	packer.pack_object(sequence);
	if(!packer.had_error())
	{
		/*
		cerr << "pack " << get_name() << get_pystr(sequence) << "\n";
		*/

		return true;
	}

	if(!Notify::ptr()->has_assert_failed())
	{
		ostringstream strm;
		PyObject *exc_type = PyExc_Exception;

		if(as_parameter() != (Parameter *)NULL)
		{
			// If it's a parameter-type field, the value may or may not be a
			// sequence.
			if(packer.had_pack_error())
			{
				strm << "Incorrect arguments to field: " << get_name()
				     << " = " << get_pystr(sequence);
				exc_type = PyExc_TypeError;
			}
			else
			{
				strm << "Value out of range on field: " << get_name()
				     << " = " << get_pystr(sequence);
				exc_type = PyExc_ValueError;
			}

		}
		else
		{
			// If it's a molecular or atomic field, the value should be a
			// sequence.
			PyObject *tuple = PySequence_Tuple(sequence);
			if(tuple == (PyObject *)NULL)
			{
				strm << "Value for " << get_name() << " not a sequence: " \
				     << get_pystr(sequence);
				exc_type = PyExc_TypeError;

			}
			else
			{
				if(packer.had_pack_error())
				{
					strm << "Incorrect arguments to field: " << get_name()
					     << get_pystr(sequence);
					exc_type = PyExc_TypeError;
				}
				else
				{
					strm << "Value out of range on field: " << get_name()
					     << get_pystr(sequence);
					exc_type = PyExc_ValueError;
				}

				Py_DECREF(tuple);
			}
		}

		string message = strm.str();
		PyErr_SetString(exc_type, message.c_str());
	}
	return false;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::unpack_args
//       Access: Published
//  Description: Unpacks the values from the packer, beginning at
//               the current point in the unpack_buffer, into a Python
//               tuple and returns the tuple.
//
//               It is assumed that the packer is currently positioned
//               on this field.
////////////////////////////////////////////////////////////////////
PyObject *Field::
unpack_args(Packer &packer) const
{
	nassertr(!packer.had_error(), NULL);
	nassertr(packer.get_current_field() == this, NULL);

	size_t start_byte = packer.get_num_unpacked_bytes();
	PyObject *object = packer.unpack_object();

	if(!packer.had_error())
	{
		// Successfully unpacked.
		/*
		cerr << "recv " << get_name() << get_pystr(object) << "\n";
		*/

		return object;
	}

	if(!Notify::ptr()->has_assert_failed())
	{
		ostringstream strm;
		PyObject *exc_type = PyExc_Exception;

		if(packer.had_pack_error())
		{
			strm << "Data error unpacking field ";
			output(strm, true);
			size_t length = packer.get_unpack_length() - start_byte;
			strm << "\nGot data (" << (int)length << " bytes):\n";
			Datagram dg(packer.get_unpack_data() + start_byte, length);
			dg.dump_hex(strm);
			size_t error_byte = packer.get_num_unpacked_bytes() - start_byte;
			strm << "Error detected on byte " << error_byte
			     << " (" << hex << error_byte << dec << " hex)";

			exc_type = PyExc_RuntimeError;
		}
		else
		{
			strm << "Value outside specified range when unpacking field "
			     << get_name() << ": " << get_pystr(object);
			exc_type = PyExc_ValueError;
		}

		string message = strm.str();
		PyErr_SetString(exc_type, message.c_str());
	}

	Py_XDECREF(object);
	return NULL;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::receive_update
//       Access: Published
//  Description: Extracts the update message out of the datagram and
//               applies it to the indicated object by calling the
//               appropriate method.
////////////////////////////////////////////////////////////////////
void Field::
receive_update(Packer &packer, PyObject *distobj) const
{
	if(as_parameter() != (Parameter *)NULL)
	{
		// If it's a parameter-type field, just store a new value on the
		// object.
		PyObject *value = unpack_args(packer);
		if(value != (PyObject *)NULL)
		{
			PyObject_SetAttrString(distobj, (char *)_name.c_str(), value);
		}
		Py_DECREF(value);

	}
	else
	{
		// Otherwise, it must be an atomic or molecular field, so call the
		// corresponding method.

		if(!PyObject_HasAttrString(distobj, (char *)_name.c_str()))
		{
			// If there's no Python method to receive this message, don't
			// bother unpacking it to a Python tuple--just skip past the
			// message.
			packer.unpack_skip();

		}
		else
		{
			// Otherwise, get a Python tuple from the args and call the Python
			// method.
			PyObject *args = unpack_args(packer);

			if(args != (PyObject *)NULL)
			{
				PyObject *func = PyObject_GetAttrString(distobj, (char *)_name.c_str());
				nassertv(func != (PyObject *)NULL);

				PyObject *result;
				{
#ifdef WITHIN_PANDA
					PStatTimer timer(((Field *)this)->_field_update_pcollector);
#endif
					result = PyObject_CallObject(func, args);
				}
				Py_XDECREF(result);
				Py_DECREF(func);
				Py_DECREF(args);
			}
		}
	}
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::client_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the client.
////////////////////////////////////////////////////////////////////
Datagram Field::
client_format_update(DOID_TYPE do_id, PyObject *args) const
{
	Packer packer;

	packer.raw_pack_uint16(CLIENT_OBJECT_UPDATE_FIELD);
	packer.raw_pack_uint32(do_id);
	packer.raw_pack_uint16(_number);

	packer.begin_pack(this);
	pack_args(packer, args);
	if(!packer.end_pack())
	{
		return Datagram();
	}

	return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::ai_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram Field::
ai_format_update(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const
{
	Packer packer;

	packer.raw_pack_uint8(1);
	packer.RAW_PACK_CHANNEL(to_id);
	packer.RAW_PACK_CHANNEL(from_id);
	packer.raw_pack_uint16(STATESERVER_OBJECT_UPDATE_FIELD);
	packer.raw_pack_uint32(do_id);
	packer.raw_pack_uint16(_number);

	packer.begin_pack(this);
	pack_args(packer, args);
	if(!packer.end_pack())
	{
		return Datagram();
	}

	return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::ai_format_update_msg_type
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update, with the msg type,
//               for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram Field::
ai_format_update_msg_type(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type,
                          PyObject *args) const
{
	Packer packer;

	packer.raw_pack_uint8(1);
	packer.RAW_PACK_CHANNEL(to_id);
	packer.RAW_PACK_CHANNEL(from_id);
	packer.raw_pack_uint16(msg_type);
	packer.raw_pack_uint32(do_id);
	packer.raw_pack_uint16(_number);

	packer.begin_pack(this);
	pack_args(packer, args);
	if(!packer.end_pack())
	{
		return Datagram();
	}

	return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON


////////////////////////////////////////////////////////////////////
//     Function: Field::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void Field::
generate_hash(HashGenerator &hashgen) const
{
	// It shouldn't be necessary to explicitly add _number to the
	// hash--this is computed based on the relative position of this
	// field with the other fields, so adding it explicitly will be
	// redundant.  However, the field name is significant.
	hashgen.add_string(_name);

	// Actually, we add _number anyway, since we need to ensure the hash
	// code comes out different in the dc_multiple_inheritance case.
	if(dc_multiple_inheritance)
	{
		hashgen.add_int(_number);
	}
}

////////////////////////////////////////////////////////////////////
//     Function: Field::pack_default_value
//       Access: Public, Virtual
//  Description: Packs the field's specified default value (or a
//               sensible default if no value is specified) into the
//               stream.  Returns true if the default value is packed,
//               false if the field doesn't know how to pack its
//               default value.
////////////////////////////////////////////////////////////////////
bool Field::
pack_default_value(PackData &pack_data, bool &) const
{
	// The default behavior is to pack the default value if we got it;
	// otherwise, to return false and let the packer visit our nested
	// elements.
	if(!_default_value_stale)
	{
		pack_data.append_data(_default_value.data(), _default_value.length());
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Field::set_name
//       Access: Public, Virtual
//  Description: Sets the name of this field.
////////////////////////////////////////////////////////////////////
void Field::
set_name(const string &name)
{
	PackerInterface::set_name(name);
	if(_dclass != (Class *)NULL)
	{
		_dclass->_dc_file->mark_inherited_fields_stale();
	}
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Field::get_pystr
//       Access: Public, Static
//  Description: Returns the string representation of the indicated
//               Python object.
////////////////////////////////////////////////////////////////////
string Field::
get_pystr(PyObject *value)
{
	if(value == NULL)
	{
		return "(null)";
	}

	PyObject *str = PyObject_Str(value);
	if(str != NULL)
	{
#if PY_MAJOR_VERSION >= 3
		string result = PyUnicode_AsUTF8(str);
#else
		string result = PyString_AsString(str);
#endif
		Py_DECREF(str);
		return result;
	}

	PyObject *repr = PyObject_Repr(value);
	if(repr != NULL)
	{
#if PY_MAJOR_VERSION >= 3
		string result = PyUnicode_AsUTF8(repr);
#else
		string result = PyString_AsString(repr);
#endif
		Py_DECREF(repr);
		return result;
	}

	if(value->ob_type != NULL)
	{
		PyObject *typestr = PyObject_Str((PyObject *)(value->ob_type));
		if(typestr != NULL)
		{
#if PY_MAJOR_VERSION >= 3
			string result = PyUnicode_AsUTF8(typestr);
#else
			string result = PyString_AsString(typestr);
#endif
			Py_DECREF(typestr);
			return result;
		}
	}

	return "(invalid object)";
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Field::refresh_default_value
//       Access: Protected
//  Description: Recomputes the default value of the field by
//               repacking it.
////////////////////////////////////////////////////////////////////
void Field::
refresh_default_value()
{
	Packer packer;
	packer.begin_pack(this);
	packer.pack_default_value();
	if(!packer.end_pack())
	{
		cerr << "Error while packing default value for " << get_name() << "\n";
	}
	else
	{
		_default_value.assign(packer.get_data(), packer.get_length());
	}
	_default_value_stale = false;
}
