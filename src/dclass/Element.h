// Filename: Element.h

#pragma once
#include "DataType.h"
namespace dclass   // open namespace
{

// Forward declaration
class Class;
class Field;

// An Element represents any part of a .dc file with a defined data structure.
class Element
{
	public:
		Element() : m_datatype(DT_invalid), m_bytesize(0)
		{
		}

		// get_size returns the size of the element in bytes.
		//     If the element is a varstring, varblob, or vararray, -1 is returned instead.
		inline sizetag_t get_size() const
		{
			return m_bytesize;
		}

		inline DataType get_type() const
		{
			return m_datatype;
		}

		inline bool has_fixed_size() const
		{
			return m_has_fixed_size;
		}

		// as_class returns the same pointer converted to a class pointer,
		//     if this is in fact a class; otherwise, returns NULL.
		virtual const Class* as_class() const
		{
			return NULL;
		}
		virtual Class* as_class()
		{
			return NULL;			
		}

		// as_field returns the same pointer converted to a field pointer,
		//     if this is in fact a field; otherwise, returns NULL.
		virtual const Field* as_field() const
		{
			return NULL;			
		}
		virtual Field* as_field()
		{
			return NULL;			
		}

	protected:
		DataType m_datatype;
		sizetag_t m_bytesize;
		bool m_has_fixed_size;
};


} // close namespace dclass
