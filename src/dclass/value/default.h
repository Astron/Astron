// Filename: default.h
#include <string> // std::string
namespace dclass   // open namespace dclass
{


// Forward declarations
class DistributedType;

// create_default_value returns a sensible default value for the given type (typically 0).
std::string create_default_value(const DistributedType*);


} // close namespace dclass