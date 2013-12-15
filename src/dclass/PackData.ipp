// Filename: PackData.ipp
// Created by: drose (15 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include <string.h>
namespace dclass   // open namespace dclass
{


////////////////////////////////////////////////////////////////////
//     Function: PackData::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
inline PackData::
PackData()
{
	m_buffer = NULL;
	m_allocated_size = 0;
	m_used_length = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
inline PackData::
~PackData()
{
	if(m_buffer != (const char *)NULL)
	{
		delete[] m_buffer;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::clear
//       Access: Published
//  Description: Empties the contents of the data (without necessarily
//               freeing its allocated memory).
////////////////////////////////////////////////////////////////////
inline void PackData::
clear()
{
	m_used_length = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::append_data
//       Access: Public
//  Description: Adds the indicated bytes to the end of the data.
////////////////////////////////////////////////////////////////////
inline void PackData::
append_data(const char *buffer, size_t size)
{
	set_used_length(m_used_length + size);
	memcpy(m_buffer + m_used_length - size, buffer, size);
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::get_write_pointer
//       Access: Public
//  Description: Adds the indicated number of bytes to the end of the
//               data without initializing them, and returns a pointer
//               to the beginning of the new data.
////////////////////////////////////////////////////////////////////
inline char *PackData::
get_write_pointer(size_t size)
{
	set_used_length(m_used_length + size);
	return m_buffer + m_used_length - size;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::append_junk
//       Access: Public
//  Description: Adds some uninitialized bytes to the end of the data.
////////////////////////////////////////////////////////////////////
inline void PackData::
append_junk(size_t size)
{
	set_used_length(m_used_length + size);
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::rewrite_data
//       Access: Public
//  Description: Changes the data at the indicated position to the
//               given value.  It is an error if there are not at
//               least position + size bytes in the data.
////////////////////////////////////////////////////////////////////
inline void PackData::
rewrite_data(size_t position, const char *buffer, size_t size)
{
	assert(position + size <= m_used_length);
	memcpy(m_buffer + position, buffer, size);
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::get_rewrite_pointer
//       Access: Public
//  Description: Returns a pointer into the middle of the data at the
//               indicated point.
////////////////////////////////////////////////////////////////////
inline char *PackData::
get_rewrite_pointer(size_t position, size_t size)
{
	assert(position + size <= m_used_length);
	return m_buffer + position;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::get_string
//       Access: Published
//  Description: Returns the data buffer as a string.  Also see
//               get_data().
////////////////////////////////////////////////////////////////////
inline std::string PackData::
get_string() const
{
	return std::string(m_buffer, m_used_length);
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::get_length
//       Access: Published
//  Description: Returns the current length of the buffer.  This is
//               the number of useful bytes stored in the buffer, not
//               the amount of memory it takes up.
////////////////////////////////////////////////////////////////////
inline size_t PackData::
get_length() const
{
	return m_used_length;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::get_data
//       Access: Public
//  Description: Returns the beginning of the data buffer.  The buffer
//               is not null-terminated, but see also get_string().
//               This may (or may not) return NULL if the buffer is
//               empty.
//
//               This may be used in conjunction with get_length() to
//               copy all of the bytes out of the buffer.
////////////////////////////////////////////////////////////////////
inline const char *PackData::
get_data() const
{
	return m_buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: PackData::take_data
//       Access: Public
//  Description: Returns the pointer to the beginning of the data
//               buffer, and transfers ownership of the buffer to the
//               caller.  The caller is now responsible for ultimately
//               freeing the returned pointer with delete[], if it is
//               non-NULL.  This may (or may not) return NULL if the
//               buffer is empty.
//
//               This also empties the PackData structure, and sets
//               its length to zero (so you should call get_length()
//               before calling this method).
////////////////////////////////////////////////////////////////////
inline char *PackData::
take_data()
{
	char *data = m_buffer;

	m_buffer = NULL;
	m_allocated_size = 0;
	m_used_length = 0;

	return data;
}


} // close namespace dclass
