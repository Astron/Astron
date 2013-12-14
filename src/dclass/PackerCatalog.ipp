// Filename: PackerCatalog.ipp
// Created by: drose (21 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
namespace dclass   // open namespace dclass
{


////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::get_begin
//       Access: Public
//  Description: Returns the beginning of the indicated field within
//               the live data.
////////////////////////////////////////////////////////////////////
inline size_t PackerCatalog::LiveCatalog::
get_begin(int n) const
{
	nassertr(n >= 0 && n < (int)_live_entries.size(), 0);
	return _live_entries[n]._begin;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::get_end
//       Access: Public
//  Description: Returns the end of the indicated field (the byte
//               position of the first following field) within the
//               live data.
////////////////////////////////////////////////////////////////////
inline size_t PackerCatalog::LiveCatalog::
get_end(int n) const
{
	nassertr(n >= 0 && n < (int)_live_entries.size(), 0);
	return _live_entries[n]._end;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::get_num_entries
//       Access: Public
//  Description: Returns the number of entries in the catalog.
////////////////////////////////////////////////////////////////////
inline int PackerCatalog::LiveCatalog::
get_num_entries() const
{
	return _catalog->get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::get_entry
//       Access: Public
//  Description: Returns the nth entry in the catalog.
////////////////////////////////////////////////////////////////////
inline const PackerCatalog::Entry &PackerCatalog::LiveCatalog::
get_entry(int n) const
{
	return _catalog->get_entry(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::find_entry_by_name
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated name, or -1 if no entry has the indicated
//               name.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int PackerCatalog::LiveCatalog::
find_entry_by_name(const string &name) const
{
	return _catalog->find_entry_by_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::LiveCatalog::find_entry_by_field
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated field, or -1 if no entry has the indicated
//               field.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int PackerCatalog::LiveCatalog::
find_entry_by_field(const PackerInterface *field) const
{
	return _catalog->find_entry_by_field(field);
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::get_num_entries
//       Access: Public
//  Description: Returns the number of entries in the catalog.
////////////////////////////////////////////////////////////////////
inline int PackerCatalog::
get_num_entries() const
{
	return _entries.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PackerCatalog::get_entry
//       Access: Public
//  Description: Returns the nth entry in the catalog.
////////////////////////////////////////////////////////////////////
inline const PackerCatalog::Entry &PackerCatalog::
get_entry(int n) const
{
	nassertr(n >= 0 && n < (int)_entries.size(), _entries[0]);
	return _entries[n];
}


} // close namespace dclass
