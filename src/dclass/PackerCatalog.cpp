// Filename: PackerCatalog.cxx
// Created by: drose (21 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "PackerCatalog.h"
#include "PackerInterface.h"
#include "Packer.h"
namespace dclass   // open namespace dclass
{


////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::Constructor
//       Access: Private
//  Description: The catalog is created only by
//               PackerInterface::get_catalog().
////////////////////////////////////////////////////////////////////
PackerCatalog::
PackerCatalog(const PackerInterface *root) : _root(root)
{
	_live_catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::Destructor
//       Access: Private
//  Description: The catalog is destroyed only by
//               ~PackerInterface().
////////////////////////////////////////////////////////////////////
PackerCatalog::
~PackerCatalog()
{
	if(_live_catalog != (LiveCatalog *)NULL)
	{
		delete _live_catalog;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::find_entry_by_name
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated name, or -1 if no entry has the indicated
//               name.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int PackerCatalog::
find_entry_by_name(const std::string &name) const
{
	EntriesByName::const_iterator ni;
	ni = _entries_by_name.find(name);
	if(ni != _entries_by_name.end())
	{
		return (*ni).second;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::find_entry_by_field
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated field, or -1 if no entry has the indicated
//               field.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int PackerCatalog::
find_entry_by_field(const PackerInterface *field) const
{
	EntriesByField::const_iterator ni;
	ni = _entries_by_field.find(field);
	if(ni != _entries_by_field.end())
	{
		return (*ni).second;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::get_live_catalog
//       Access: Public
//  Description: Returns a LiveCatalog object indicating the positions
//               within the indicated data record of each field within
//               the catalog.  If the catalog's fields are all
//               fixed-width, this may return a statically-allocated
//               LiveCatalog object that is the same for all data
//               records; otherwise, it will allocate a new
//               LiveCatalog object that must be freed with a later
//               call to release_live_catalog().
////////////////////////////////////////////////////////////////////
const PackerCatalog::LiveCatalog *PackerCatalog::
get_live_catalog(const char *data, size_t length) const
{
	if(_live_catalog != (LiveCatalog *)NULL)
	{
		// Return the previously-allocated live catalog; it will be the
		// same as this one since it's based on a fixed-length field.
		return _live_catalog;
	}

	LiveCatalog *live_catalog = new LiveCatalog;
	live_catalog->_catalog = this;
	live_catalog->_live_entries.reserve(_entries.size());
	LiveCatalogEntry zero_entry;
	zero_entry._begin = 0;
	zero_entry._end = 0;
	for(size_t i = 0; i < _entries.size(); i++)
	{
		live_catalog->_live_entries.push_back(zero_entry);
	}

	Packer packer;
	packer.set_unpack_data(data, length, false);
	packer.begin_unpack(_root);
	r_fill_live_catalog(live_catalog, packer);
	bool okflag = packer.end_unpack();

	if(!okflag)
	{
		delete live_catalog;
		return NULL;
	}

	// If our root field has a fixed structure, then the live catalog
	// will always be the same every time, so we might as well keep
	// this one around as an optimization.
	((PackerCatalog *)this)->_live_catalog = live_catalog;

	return live_catalog;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::release_live_catalog
//       Access: Public
//  Description: Releases the LiveCatalog object that was returned by
//               an earlier call to get_live_catalog().  If this
//               represents a newly-allocated live catalog, it will
//               free it; otherwise, it will do nothing.
//
//               It is therefore always correct (and necessary) to
//               match a call to get_live_catalog() with a later call
//               to release_live_catalog().
////////////////////////////////////////////////////////////////////
void PackerCatalog::
release_live_catalog(const PackerCatalog::LiveCatalog *live_catalog) const
{
	if(live_catalog != _live_catalog)
	{
		delete(LiveCatalog *)live_catalog;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::add_entry
//       Access: Private
//  Description: Called only by PackerInterface::r_fill_catalog(),
//               this adds a new entry to the catalog.
////////////////////////////////////////////////////////////////////
void PackerCatalog::
add_entry(const std::string &name, const PackerInterface *field,
          const PackerInterface *parent, int field_index)
{
	Entry entry;
	entry._name = name;
	entry._field = field;
	entry._parent = parent;
	entry._field_index = field_index;

	int entry_index = (int)_entries.size();
	_entries.push_back(entry);
	_entries_by_field.insert(EntriesByField::value_type(field, entry_index));

	// Add an entry for the fully-qualified field name
	// (e.g. dna.topTex).  If there was another entry for this name
	// previously, completely replace it--the fully-qualified name is
	// supposed to be unique and trumps the local field names (which are
	// not necessarily unique).
	_entries_by_name[name] = entry_index;

	// We'll also add an entry for the local field name, for the user's
	// convenience.  This won't override a fully-qualified name that
	// might already have been recorded, and a fully-qualified name
	// discovered later that conflicts with this name will replace it.
	std::string local_name = field->get_name();
	if(local_name != name)
	{
		_entries_by_name.insert(EntriesByName::value_type(local_name, entry_index));
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::r_fill_catalog
//       Access: Private
//  Description: Called by PackerInterface to recursively fill up a
//               newly-allocated reference catalog.
////////////////////////////////////////////////////////////////////
void PackerCatalog::
r_fill_catalog(const std::string &name_prefix, const PackerInterface *field,
               const PackerInterface *parent, int field_index)
{
	std::string next_name_prefix = name_prefix;

	if(parent != (const PackerInterface *)NULL && !field->get_name().empty())
	{
		// Record this entry in the catalog.
		next_name_prefix += field->get_name();
		add_entry(next_name_prefix, field, parent, field_index);

		next_name_prefix += ".";
	}

	// Add any children.
	if(field->has_nested_fields())
	{
		int num_nested = field->get_num_nested_fields();
		// It's ok if num_nested is -1.
		for(int i = 0; i < num_nested; i++)
		{
			PackerInterface *nested = field->get_nested_field(i);
			if(nested != (PackerInterface *)NULL)
			{
				r_fill_catalog(next_name_prefix, nested, field, i);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::r_fill_live_catalog
//       Access: Private
//  Description: Recursively walks through all of the fields on the
//               catalog and fills the live catalog with the
//               appropriate offsets.
////////////////////////////////////////////////////////////////////
void PackerCatalog::
r_fill_live_catalog(LiveCatalog *live_catalog, Packer &packer) const
{
	const PackerInterface *current_field = packer.get_current_field();

	int field_index = live_catalog->find_entry_by_field(current_field);
	if(field_index >= 0)
	{
		assert(field_index < (int)live_catalog->_live_entries.size());
		live_catalog->_live_entries[field_index]._begin = packer.get_num_unpacked_bytes();
	}

	if(packer.has_nested_fields() &&
	        (packer.get_pack_type() != PT_string && packer.get_pack_type() != PT_blob))
	{
		packer.push();
		while(packer.more_nested_fields())
		{
			r_fill_live_catalog(live_catalog, packer);
		}
		packer.pop();

	}
	else
	{
		packer.unpack_skip();
	}

	if(field_index >= 0)
	{
		live_catalog->_live_entries[field_index]._end = packer.get_num_unpacked_bytes();
	}
}


} // close namespace dclass
