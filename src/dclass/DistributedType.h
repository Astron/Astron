// Filename: DistributedType.h
#pragma once
#include "DataType.h"
namespace dclass   // open namespace dclass
{


// Forward declaration
class Struct;
class Field;

// An DistributedType represents any part of a .dc file which can have 
class DistributedType
{
    protected:
        // null constructor
        DistributedType() : m_datatype(DT_invalid), m_bytesize(0), m_has_fixed_size(true)
        {
        }

        // copy constructor
        DistributedType(const DistributedType& copy) : m_datatype(copy.m_datatype),
            m_bytesize(copy.m_bytesize), m_has_fixed_size(copy.m_has_fixed_size)
        {
        }

    public:
        // get_size returns the size of the DistributedType in bytes.
        //     If the type has variable size, 0 is returned instead.
        inline sizetag_t get_size() const
        {
            return m_bytesize;
        }

        inline DataType get_datatype() const
        {
            return m_datatype;
        }

        inline bool has_fixed_size() const
        {
            return m_has_fixed_size;
        }

        // as_struct returns the same pointer converted to a struct pointer,
        //     if this is in fact a struct; otherwise, returns NULL.
        virtual const Struct* as_struct() const
        {
            return NULL;
        }
        virtual Struct* as_struct()
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
