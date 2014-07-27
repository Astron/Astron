#include <set>
#include <map>
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"

// Convenience typedefs
typedef std::set<const dclass::Field*, dclass::FieldPtrComp> FieldSet;
typedef std::map<const dclass::Field*, std::vector<uint8_t>, dclass::FieldPtrComp> FieldValues;
typedef std::unordered_map<const dclass::Field*, std::vector<uint8_t> > UnorderedFieldValues;
