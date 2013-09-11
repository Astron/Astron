#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"
#include <list>

// A field_t object is a <DCField, unpacked_data> pair
// TODO: Think about easier/more-efficient/etc ways to pass this data.
typedef std::pair<DCField*, std::string> field_t;
typedef std::list<field_t> FieldList;

class Database
{
public:
	virtual void create_object(unsigned int do_id, FieldList& in) = 0;
	virtual void delete_object(unsigned int do_id) = 0;
	virtual void select_object(unsigned int do_id, FieldList& out) = 0;
	virtual void update_object(unsigned int do_id, FieldList& in) = 0;
	virtual void update_object_if_equals(unsigned int do_id, FieldList& in, FieldList& equals) = 0;

	virtual void select_query(/* TODO: Args */) = 0;
	virtual void update_query(/* TODO: Args */) = 0;
	virtual void delete_query(/* TODO: Args */) = 0;


protected:
	Database(DatabaseConfig dbconfig) : m_dbconfig(dbconfig) {}

	DatabaseConfig m_dbconfig;
};