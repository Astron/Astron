#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"
#include "util/DatagramIterator.h"

class Database
{
public:
	virtual void create_object(unsigned int do_id,
							   unsigned short dc_id,
							   unsigned short field_count,
							   DatagramIterator& data) = 0;
	virtual void delete_object(unsigned int do_id) = 0;
	virtual void select_object(unsigned int do_id /* TODO: Return values */) = 0;
	virtual void update_object(unsigned int do_id,
							   unsigned short field_count,
							   DatagramIterator& data) = 0;
	virtual void update_object_if_equals(unsigned int do_id,
										 unsigned short field_count,
										 DatagramIterator& data) = 0;

	virtual void select_query(/* TODO: Args */) = 0;
	virtual void update_query(/* TODO: Args */) = 0;
	virtual void delete_query(/* TODO: Args */) = 0;

protected:
	Database(DatabaseConfig dbconfig) : m_dbconfig(dbconfig) {}

	DatabaseConfig m_dbconfig;
};