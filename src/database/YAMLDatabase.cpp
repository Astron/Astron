#include "Database.h"
#include "DatabaseFactory.h"

class YAMLDatabase : public Database
{
public:
	YAMLDatabase(DatabaseConfig dbconfig) : Database(dbconfig) {}

	~YAMLDatabase() {}

	void create_object(unsigned int do_id, FieldList& in) {}
	void delete_object(unsigned int do_id) {}
	void select_object(unsigned int do_id, FieldList& out) {}
	void update_object(unsigned int do_id, FieldList& in) {}
	void update_object_if_equals(unsigned int do_id, FieldList& in, FieldList& equals) {}

	void select_query(/* TODO: Args */) {}
	void update_query(/* TODO: Args */) {}
	void delete_query(/* TODO: Args */) {}
};

DatabaseItem<YAMLDatabase> yaml_fact("yaml");