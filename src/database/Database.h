#pragma once
#include "util/DatagramIterator.h"
#include <list>

// A field_t object is a <DCField, unpacked_data> pair
// TODO: Think about easier/more-efficient/etc ways to pass this data.
typedef std::pair<DCField*, std::string> field_t;
typedef std::list<field_t> FieldList;

class Database
{
public:
	virtual void create_object(unsigned int do_id, FieldList& in);
	virtual void delete_object(unsigned int do_id);
	virtual void select_object(unsigned int do_id, FieldList& out);
	virtual void update_object(unsigned int do_id, FieldList& in);
	virtual void update_object_if_equals(unsigned int do_id, FieldList& in, FieldList& equals);

	virtual void select_query(/* TODO: Args */);
	virtual void update_query(/* TODO: Args */);
	virtual void delete_query(/* TODO: Args */);
};