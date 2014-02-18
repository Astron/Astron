#include "DatabaseBackend.h"
#include "DBBackendFactory.h"
#include "DatabaseServer.h"

#include "core/global.h"
#include "dclass/value/parse.h"
#include "dclass/value/format.h"
#include "dclass/dc/Class.h"
#include "dclass/dc/Field.h"

#include <soci.h>
#include <boost/icl/interval_set.hpp>

using namespace std;
using namespace soci;
using namespace dclass;

typedef boost::icl::discrete_interval<doid_t> interval_t;
typedef boost::icl::interval_set<doid_t> set_t;

static ConfigVariable<string> database_name("database", "null", db_backend_config);
static ConfigVariable<string> session_user("username", "null", db_backend_config);
static ConfigVariable<string> session_passwd("password", "null", db_backend_config);

class SociSQLDatabase : public DatabaseBackend
{
	public:
		SociSQLDatabase(ConfigNode dbeconfig, doid_t min_id, doid_t max_id) :
			DatabaseBackend(dbeconfig, min_id, max_id), m_min_id(min_id), m_max_id(max_id),
			m_backend(db_backend_type.get_rval(dbeconfig)),
			m_db_name(database_name.get_rval(dbeconfig)),
			m_sess_user(session_user.get_rval(dbeconfig)),
			m_sess_passwd(session_passwd.get_rval(dbeconfig))
		{
			stringstream log_name;
			log_name << "Database-" << m_backend << "(Range: [" << min_id << ", " << max_id << "])";
			m_log = new LogCategory(m_backend, log_name.str());

			connect();
			check_tables();
			check_classes();
			check_ids();
		}

		doid_t create_object(const ObjectData& dbo)
		{
			string field_name;
			vector<uint8_t> field_value;
			const Class *dcc = g_dcf->get_class_by_id(dbo.dc_id);
			bool storable = is_storable(dbo.dc_id);

			uint32_t do_id = pop_next_id();
			if(!do_id)
			{
				return 0;
			}

			try
			{
				m_sql.begin(); // Start transaction
				m_sql << "INSERT INTO objects VALUES (" << do_id << "," << dbo.dc_id << ");";

				if(storable)
				{
					// TODO: This would probably be a lot faster if it was all one statement.
					//       Go ahead and simplify to one statement if you see a good way to do so.
					m_sql << "INSERT INTO fields_" << dcc->get_name() << "(object_id)"
					      " VALUES(" << do_id << ");";
					set_fields_in_table(do_id, dcc, dbo.fields);
				}

				m_sql.commit(); // End transaction
			}
			catch(const exception &e)
			{
				m_sql.rollback(); // Revert transaction
				return 0;
			}

			return do_id;
		}
		void delete_object(doid_t do_id)
		{
			bool storable = false;
			const Class* dcc = get_class(do_id);
			if(dcc)
			{
				// Note: needs to be called outside the transaction so it doesn't prevent deletion
				//       of the object in the `objects` table
				storable = is_storable(dcc->get_id());
			}

			m_log->debug() << "Deleting object with id " << do_id << "..." << endl;
			m_sql << "DELETE FROM objects WHERE id=" << do_id;

			if(dcc && storable)
			{
				m_log->trace() << "... object has stored field, also deleted." << endl;
				m_sql << "DELETE FROM fields_" << dcc->get_name() << " WHERE object_id=:id;", use(do_id);
			}

			push_id(do_id);
		}
		bool get_object(doid_t do_id, ObjectData& dbo)
		{
			m_log->trace() << "Getting object with id" << do_id << endl;

			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}
			dbo.dc_id = dcc->get_id();

			bool stored = is_storable(dcc->get_id());
			if(stored)
			{
				get_all_from_table(do_id, dcc, dbo.fields);
			}

			return true;
		}
		const Class* get_class(doid_t do_id)
		{
			int dc_id = -1;
			indicator ind;

			try
			{
				m_sql << "SELECT class_id FROM objects WHERE id=" << do_id << ";", into(dc_id, ind);
			}
			catch(const exception &e)
			{
				return NULL;
			}

			if(ind != i_ok || dc_id == -1)
			{
				return NULL;
			}

			return g_dcf->get_class_by_id(dc_id);
		}
		void del_field(doid_t do_id, const Field* field)
		{
			const Class *dcc = get_class(do_id);
			bool storable = is_storable(dcc->get_id());

			if(storable)
			{
				vector<const Field*> fields;
				fields.push_back(field);
				del_fields_in_table(do_id, dcc, fields);
			}
		}
		void del_fields(doid_t do_id, const vector<const Field*> &fields)
		{
			const Class *dcc = get_class(do_id);
			bool storable = is_storable(dcc->get_id());

			if(storable)
			{
				del_fields_in_table(do_id, dcc, fields);
			}
		}
		void set_field(doid_t do_id, const Field* field, const vector<uint8_t> &value)
		{
			const Class *dcc = get_class(do_id);
			bool storable = is_storable(dcc->get_id());

			if(storable)
			{
				map<const Field*, vector<uint8_t> > fields;
				fields[field] = value;
				try
				{
					m_sql.begin(); // Start transaction
					set_fields_in_table(do_id, dcc, fields);
					m_sql.commit(); // End transaction
				}
				catch(const exception &e)
				{
					m_sql.rollback(); // Revert transaction
				}
			}
		}
		void set_fields(doid_t do_id, const map<const Field*, vector<uint8_t> > &fields)
		{
			const Class *dcc = get_class(do_id);
			bool storable = is_storable(dcc->get_id());

			if(storable)
			{
				try
				{
					m_sql.begin(); // Start transaction
					set_fields_in_table(do_id, dcc, fields);
					m_sql.commit(); // End transaction
				}
				catch(const exception &e)
				{
					m_sql.rollback(); // Revert transaction
				}
			}
		}
		bool set_field_if_empty(doid_t do_id, const Field* field, vector<uint8_t> &value)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				value.clear();
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				value.clear();
				return false; // Class has no database fields
			}

			if(!field->has_keyword("db"))
			{
				value.clear();
				return false;
			}

			string val;
			indicator ind;
			m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
			      << " WHERE object_id=" << do_id << ";", into(val, ind);
			if(ind != i_null)
			{
				bool parse_err;
				string packed_data = parse_value(field->get_type(), val, parse_err);
				if(parse_err)
				{
					m_log->error() << "Failed parsing value for field '" << field->get_name()
					               << "' of object " << do_id << "' from database.\n";
					return false;
				}
				value = vector<uint8_t>(packed_data.begin(), packed_data.end());
				return false;
			}

			val = format_value(field->get_type(), value);
			m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << field->get_name()
			      << "='" << val << "' WHERE object_id=" << do_id << ";";
			return true;
		}
		bool set_fields_if_empty(doid_t do_id, map<const Field*, vector<uint8_t> > &values)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				values.clear();
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				values.clear();
				return false; // Class has no database fields
			}

			bool failed = false;
			string value;
			indicator ind;
			try
			{
				m_sql.begin(); // Start transaction
				for(auto it = values.begin(); it != values.end(); ++it)
				{
					const Field* field = it->first;
					if(field->has_keyword("db"))
					{
						m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
						      << " WHERE object_id=" << do_id << ";", into(value, ind);
						if(ind != i_null)
						{
							bool parse_err;
							failed = true;
							string packed_data = parse_value(field->get_type(), value, parse_err);
							if(parse_err)
							{
								m_log->error() << "Failed parsing value for field '" << field->get_name()
								               << "' of object " << do_id << "' from database.\n";
								continue;
							}
							values[field] = vector<uint8_t>(packed_data.begin(), packed_data.end());
							continue;
						}

						value = format_value(it->first->get_type(), it->second);
						m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << field->get_name()
						      << "='" << value << "' WHERE object_id=" << do_id << ";";
					}
				}

				if(failed)
				{
					m_sql.rollback(); // Revert transaction
				}
				else
				{
					m_sql.commit(); // End transaction
				}
			}
			catch(const exception &e)
			{
				m_sql.rollback(); // Revert transaction
				values.clear();
				return false;
			}
		}
		bool set_field_if_equals(doid_t do_id, const Field* field, const vector<uint8_t> &equal,
		                         vector<uint8_t> &value)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				return false; // Class has no database fields
			}

			if(!field->has_keyword("db"))
			{
				return false;
			}

			string val;
			indicator ind;
			m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
			      << " WHERE object_id=" << do_id << ";", into(val, ind);
			if(ind != i_ok)
			{
				value.clear();
				return false;
			}

			string eql = format_value(field->get_type(), equal);
			if(val != eql)
			{
				bool parse_err;
				val = parse_value(field->get_type(), val, parse_err);
				if(parse_err)
				{
					m_log->error() << "Failed parsing value for field '" << field->get_name()
					               << "' of object " << do_id << "' from database.\n";
					return false;
				}
				value = vector<uint8_t>(val.begin(), val.end());
				return false;
			}

			val = format_value(field->get_type(), value);
			m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << field->get_name()
			      << "='" << val << "' WHERE object_id=" << do_id << ";";
			return true;
		}
		bool set_fields_if_equals(doid_t do_id, const map<const Field*, vector<uint8_t> > &equals,
		                          map<const Field*, vector<uint8_t> > &values)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				return false; // Class has no database fields
			}

			bool failed = false;
			string value;
			indicator ind;
			vector<const Field*> null_fields;
			try
			{
				m_sql.begin(); // Start transaction
				for(auto it = equals.begin(); it != equals.end(); ++it)
				{
					const Field* field = it->first;
					if(field->has_keyword("db"))
					{
						m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
						      << " WHERE object_id=" << do_id << ";", into(value, ind);
						if(ind != i_ok)
						{
							null_fields.push_back(field);
							failed = true;
							continue;
						}

						string equal = format_value(field->get_type(), it->second);
						if(value == equal)
						{
							string insert = format_value(field->get_type(), values[field]);
							m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << field->get_name()
							      << "='" << insert << "' WHERE object_id=" << do_id << ";";
						}
						else
						{
							failed = true;
						}

						bool parse_err;
						value = parse_value(field->get_type(), value, parse_err);
						if(parse_err)
						{
							m_log->error() << "Failed parsing value for field '" << field->get_name()
							               << "' of object " << do_id << "' from database.\n";
							continue;
						}
						values[field] = vector<uint8_t>(value.begin(), value.end());
					}
				}

				if(failed)
				{
					for(auto it = null_fields.begin(); it != null_fields.end(); ++it)
					{
						values.erase(*it);
					}
					m_sql.rollback(); // Revert transaction
					return false;
				}
				else
				{
					m_sql.commit(); // End transaction
					return true;
				}
			}
			catch(const exception &e)
			{
				m_sql.rollback(); // Revert transaction
				values.clear();
				return false;
			}
		}

		bool get_field(doid_t do_id, const Field* field, vector<uint8_t> &value)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				return false; // Class has no database fields
			}

			vector<const Field*> fields;
			fields.push_back(field);
			map<const Field*, vector<uint8_t> > values;

			get_fields_from_table(do_id, dcc, fields, values);

			auto val_it = values.find(field);
			if(val_it == values.end())
			{
				return false;
			}

			value = val_it->second;

			return true;
		}
		bool get_fields(doid_t do_id, const vector<const Field*> &fields,
		                map<const Field*, vector<uint8_t> > &values)
		{
			// Get class from the objects table
			const Class* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}

			bool stored = is_storable(dcc->get_id());
			if(!stored)
			{
				return false; // Class has no database fields
			}

			get_fields_from_table(do_id, dcc, fields, values);

			return true;
		}

	protected:
		void connect()
		{
			// Prepare database, username, password, etc for connection
			stringstream connstring;
			if(m_backend == "postgresql")
			{
				connstring << "dbname=" << m_db_name;
			}
			else if(m_backend == "mysql")
			{
				connstring << "db=" << m_db_name << " "
				           << "user=" << m_sess_user << " "
				           << "pass='" << m_sess_passwd << "'";
			}
			else if(m_backend == "sqlite3")
			{
				connstring << m_db_name;
			}

			// Connect to database
			m_sql.open(m_backend, connstring.str());
		}

		void check_tables()
		{
			m_sql << "CREATE TABLE IF NOT EXISTS objects ("
			      "id INT NOT NULL PRIMARY KEY, class_id INT NOT NULL);";
			//"CONSTRAINT check_object CHECK (id BETWEEN " << m_min_id << " AND " << m_max_id << "));";
			m_sql << "CREATE TABLE IF NOT EXISTS classes ("
			      "id INT NOT NULL PRIMARY KEY, name VARCHAR(32) NOT NULL,"
			      "storable BOOLEAN NOT NULL);";//, CONSTRAINT check_class CHECK (id BETWEEN 0 AND "
			//<< g_dcf->get_num_types()-1 << "));";
		}

		void check_classes()
		{
			int dc_id;
			uint8_t storable;
			string dc_name;

			// Prepare sql statements
			statement get_row_by_id = (m_sql.prepare << "SELECT name FROM classes WHERE id=:id",
			                           into(dc_name), use(dc_id));
			statement insert_class = (m_sql.prepare << "INSERT INTO classes VALUES (:id,:name,:stored)",
			                          use(dc_id), use(dc_name), use(storable));

			// For each class, verify an entry exists and has the correct name and value
			for(unsigned int i = 0; i < g_dcf->get_num_classes(); ++i)
			{
				dc_id = g_dcf->get_class(i)->get_id();
				get_row_by_id.execute(true);
				if(m_sql.got_data())
				{
					check_class(dc_id, dc_name);
				}
				else
				{
					const Class* dcc = g_dcf->get_class(i);

					// Create fields table for the class
					storable = create_fields_table(dcc);

					// Create class row in classes table
					dc_name = dcc->get_name();
					insert_class.execute(true);
				}
			}
		}
		void check_ids()
		{
			// Set all ids as free ids
			m_free_ids = set_t();
			m_free_ids += interval_t::closed(m_min_id, m_max_id);

			doid_t id;

			// Get all ids from the database at once
			statement st = (m_sql.prepare << "SELECT id FROM objects;", into(id));
			st.execute();

			// Iterate through the result set, removing used ids from the free ids
			while(st.fetch())
			{
				m_free_ids -= interval_t::closed(id, id);
			}
		}

		doid_t pop_next_id()
		{
			// Check to make sure any free ids exist
			if(!m_free_ids.size())
			{
				return INVALID_DO_ID;
			}

			// Get next available id from m_free_ids set
			interval_t first = *m_free_ids.begin();
			doid_t id = first.lower();
			if(!(first.bounds().bits() & BOOST_BINARY(10)))
			{
				id += 1;
			}

			// Check if its within range
			if(id > m_max_id)
			{
				return INVALID_DO_ID;
			}

			// Remove it from the free ids
			m_free_ids -= interval_t::closed(id, id);

			return id;
		}

		void push_id(doid_t id)
		{
			m_free_ids += interval_t::closed(id, id);
		}
	private:
		doid_t m_min_id, m_max_id;
		string m_backend, m_db_name;
		string m_sess_user, m_sess_passwd;
		session m_sql;
		set_t m_free_ids;
		LogCategory* m_log;

		void check_class(uint16_t id, string name)
		{
			const Class* dcc = g_dcf->get_class_by_id(id);
			if(name != dcc->get_name())
			{
				// TODO: Try and update the database instead of exiting
				m_log->fatal() << "Class name '" << dcc->get_name() << "' from File does not match"
				               " name '" << name << "' in database, for dc_id " << id << endl;
				m_log->fatal() << "Database must be rebuilt." << endl;
				exit(1);
			}

			// TODO: Check class_fields table exists

		}

		// returns true if class has db fields
		bool create_fields_table(const Class* dcc)
		{
			stringstream ss;
			ss << "CREATE TABLE IF NOT EXISTS fields_" << dcc->get_name()
			   << "(object_id INT NOT NULL PRIMARY KEY";

			int db_field_count = 0;
			for(int i = 0; i < dcc->get_num_fields(); ++i)
			{
				const Field* field = dcc->get_field(i);
				if(field->has_keyword("db") && !field->as_molecular())
				{
					db_field_count += 1;
					// TODO: Store SimpleParameters and fields with 1 SimpleParameter
					//       as a simpler type.
					// NOTE: This might be a lot easier if the Parser was modified
					//       such that atomic fields containing only 1 SimpleParameter
					//       element are initialized as a SimpleField subclass of AtomicField.
					// TODO: Also see if you can't find a convenient way to get the max length of
					//       for example a string field, and use a VARCHAR(len) instead of TEXT.
					//       Same for blobs with VARBINARY.
					ss << "," << field->get_name() << " TEXT";
				}
			}

			if(db_field_count > 0)
			{
				ss << ");";
				m_sql << ss.str();
				return true;
			}

			return false;
		}

		bool is_storable(uint16_t dc_id)
		{
			uint8_t storable;
			m_sql << "SELECT storable FROM classes WHERE id=:id", into(storable), use(dc_id);
			return storable;
		}

		void get_all_from_table(doid_t id, const Class* dcc, map<const Field*, vector<uint8_t> > &fields)
		{
			string value;
			indicator ind;
			for(int i = 0; i < dcc->get_num_fields(); ++i)
			{
				const Field* field = dcc->get_field(i);
				if(field->has_keyword("db"))
				{
					m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
					      << " WHERE object_id=" << id << ";", into(value, ind);

					if(ind == i_ok)
					{
						bool parse_err;
						string packed_data = parse_value(field->get_type(), value, parse_err);
						if(parse_err)
						{
							m_log->error() << "Failed parsing value for field '" << field->get_name()
							               << "' of object " << id << "' from database.\n";
							continue;
						}
						fields[field] = vector<uint8_t>(packed_data.begin(), packed_data.end());
					}
				}
			}
		}

		void get_fields_from_table(doid_t id, const Class* dcc, const vector<const Field*> &fields,
		                           map<const Field*, vector<uint8_t> > &values)
		{
			string value;
			indicator ind;
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				const Field* field = *it;
				if(field->has_keyword("db"))
				{
					m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
					      << " WHERE object_id=" << id << ";", into(value, ind);

					if(ind == i_ok)
					{
						bool parse_err;
						string packed_data = parse_value(field->get_type(), value, parse_err);
						if(parse_err)
						{
							m_log->error() << "Failed parsing value for field '" << field->get_name()
							               << "' of object " << id << "' from database.\n";
							continue;
						}
						values[field] = vector<uint8_t>(packed_data.begin(), packed_data.end());
					}
				}
			}
		}

		void set_fields_in_table(doid_t id, const Class* dcc,
		                         const map<const Field*, vector<uint8_t> > &fields)
		{
			string name, value;
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				if(it->first->has_keyword("db"))
				{
					name = it->first->get_name();
					value = format_value(it->first->get_type(), it->second);
					m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << name << "='" << value
					      << "' WHERE object_id=" << id << ";";
				}
			}
		}

		void del_fields_in_table(doid_t id, const Class* dcc, const vector<const Field*> &fields)
		{
			string name;
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				const Field* field = *it;
				if(field->has_keyword("db"))
				{
					m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << field->get_name()
					      << "=NULL WHERE object_id=" << id << ";";
				}
			}
		}
};

DBBackendFactoryItem<SociSQLDatabase> mysql_factory("mysql");
DBBackendFactoryItem<SociSQLDatabase> postgresql_factory("postgresql");
DBBackendFactoryItem<SociSQLDatabase> sqlite_factory("sqlite3");
