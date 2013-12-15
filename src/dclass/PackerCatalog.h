// Filename: PackerCatalog.h
// Created by: drose (21 Jun, 2004)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include <map> // for std::map
#include <vector> // for std::vector
#include <assert.h>
namespace dclass   // open namespace dclass
{

class PackerInterface;
class Packer;

////////////////////////////////////////////////////////////////////
//       Class : PackerCatalog
// Description : This object contains the names of all of the nested
//               fields available within a particular field.  It is
//               created on demand when a catalog is first requested
//               from a particular field; its ownership is retained by
//               the field so it must not be deleted.
////////////////////////////////////////////////////////////////////
class PackerCatalog
{
	private:
		PackerCatalog(const PackerInterface *root);
		PackerCatalog(const PackerCatalog &copy);
		~PackerCatalog();

	public:
		// The Entry class records the static catalog data: the name of each
		// field and its relationship to its parent.
		class Entry
		{
			public:
				std::string _name;
				const PackerInterface *_field;
				const PackerInterface *_parent;
				int _field_index;
		};

		// The LiveCatalog class adds the dynamic catalog data: the actual
		// location of each field within the data record.  This might be
		// different for different data records (since some data fields have
		// a dynamic length).
		class LiveCatalogEntry
		{
			public:
				size_t _begin;
				size_t _end;
		};
		class LiveCatalog
		{
			public:
				inline size_t get_begin(int n) const;
				inline size_t get_end(int n) const;

				inline int get_num_entries() const;
				inline const Entry &get_entry(int n) const;
				inline int find_entry_by_name(const std::string &name) const;
				inline int find_entry_by_field(const PackerInterface *field) const;

			private:
				typedef std::vector<LiveCatalogEntry> LiveEntries;
				LiveEntries _live_entries;

				const PackerCatalog *_catalog;
				friend class PackerCatalog;
		};

		inline int get_num_entries() const;
		inline const Entry &get_entry(int n) const;
		int find_entry_by_name(const std::string &name) const;
		int find_entry_by_field(const PackerInterface *field) const;

		const LiveCatalog *get_live_catalog(const char *data, size_t length) const;
		void release_live_catalog(const LiveCatalog *live_catalog) const;

	private:
		void add_entry(const std::string &name, const PackerInterface *field,
		               const PackerInterface *parent, int field_index);

		void r_fill_catalog(const std::string &name_prefix, const PackerInterface *field,
		                    const PackerInterface *parent, int field_index);
		void r_fill_live_catalog(LiveCatalog *live_catalog, Packer &packer) const;


		const PackerInterface *_root;
		LiveCatalog *_live_catalog;

		typedef std::vector<Entry> Entries;
		Entries _entries;

		typedef std::map<std::string, int> EntriesByName;
		EntriesByName _entries_by_name;

		typedef std::map<const PackerInterface *, int> EntriesByField;
		EntriesByField _entries_by_field;

		friend class PackerInterface;
};


} // close namespace dclass

#include "PackerCatalog.ipp"
