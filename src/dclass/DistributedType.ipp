// Filename: DistributedType.ipp
namespace dclass   // open namespace dclass
{


// null constructor
inline DistributedType::DistributedType() :
    m_type(INVALID), m_size(0)
{
}

// get_type returns the type's fundamental type as an integer constant.
inline Type DistributedType::get_type() const
{
    return m_type;
}

// has_fixed_size returns true if the DistributedType has a fixed size in bytes.
inline bool DistributedType::has_fixed_size() const
{
    return (m_size != 0);
}
// get_size returns the size of the DistributedType in bytes or 0 if it is variable.
inline sizetag_t DistributedType::get_size() const
{
    return m_size;
}


} // close namespace dclass
