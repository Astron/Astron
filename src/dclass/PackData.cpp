// Filename: PackData.cxx
// Created by: drose (15 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "PackData.h"
namespace dclass   // open namespace dclass
{


static const size_t extra_size = 50;

////////////////////////////////////////////////////////////////////
//     Function: DCPackData::set_used_length
//       Access: Private
//  Description: Ensures that the buffer has at least size bytes, and
//               sets the _used_length to the indicated value; grows
//               the buffer if it does not.
////////////////////////////////////////////////////////////////////
void PackData::
set_used_length(size_t size)
{
	if(size > m_allocated_size)
	{
		m_allocated_size = size + size + extra_size;
		char *new_buf = new char[m_allocated_size];
		if(m_used_length > 0)
		{
			memcpy(new_buf, m_buffer, m_used_length);
		}
		if(m_buffer != NULL)
		{
			delete[] m_buffer;
		}
		m_buffer = new_buf;
	}

	m_used_length = size;
}


} // close namespace dclass
