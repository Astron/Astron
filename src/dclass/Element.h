// Filename: Element.h

#pragma once
#include "DataType.h"
namespace dclass   // open namespace
{


// An Element represents any part of a .dc file with a defined data structure.
class Element
{
	public:
		Element() : m_datatype(DT_invalid), m_bytesize(-1)
		{
		}

		// get_size returns the size of the element in bytes.
		//     If the size is variable, it returns -1 instead.
		inline length_tag_t get_size() const
		{
			return m_bytesize;
		}

		inline DataType get_type() const
		{
			return m_datatype;
		}

	protected:
		DataType m_datatype;
		length_tag_t m_bytesize;
};


} // close namespace dclass
