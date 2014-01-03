// Filename: Hashable.ipp
namespace dclass   // open namespace dclass
{


inline uint32_t Hashable::get_hash() const
{
	HashGenerator hashgen;
	generate_hash(hashgen);
	return hashgen.get_hash();
}


} // close namespace dclass
