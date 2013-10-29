#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "core/global.h"
#include "dcparser/hashGenerator.h"
#include "dcparser/dcFile.h"
#include "dcparser/dcClass.h"
#include "dcparser/dcField.h"

#include <soci.h>
#include <boost/icl/interval_set.hpp>

using namespace std;
using namespace soci;

typedef boost::icl::discrete_interval<uint32_t> interval_t;
typedef boost::icl::interval_set<uint32_t> set_t;

static ConfigVariable<std::string> engine_type("type", "null");
static ConfigVariable<std::string> database_name("database", "null");
static ConfigVariable<std::string> session_user("username", "null");
static ConfigVariable<std::string> session_passwd("password", "null");

class SociSQLEngine : public IDatabaseEngine
{
	public:
		SociSQLEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
			IDatabaseEngine(dbeconfig, min_id, max_id), m_min_id(min_id), m_max_id(max_id),
			m_backend(engine_type.get_rval(dbeconfig)),
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

		uint32_t create_object(const DatabaseObject& dbo)
		{
			string field_name;
			vector<uint8_t> field_value;
			DCClass *dcc = g_dcf->get_class(dbo.dc_id);
			bool storable = is_storable(dbo.dc_id);

			uint32_t do_id = pop_next_id();
			if(!do_id)
			{
				return 0;
			}

			try {
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
			catch (const exception &e)
			{
				//m_sql.rollback(); // Revert transaction
				//return 0;
			}

			return do_id;
		}
		void delete_object(uint32_t do_id)
		{
			bool storable = false;
			DCClass* dcc = get_class(do_id);
			if(dcc)
			{   // Note: needs to be called outside the transaction so it doesn't prevent deletion
			    //       of the object in the `objects` table
				storable = is_storable(dcc->get_number());
			}

			m_log->debug() << "Deleting object with id " << do_id << "..." << endl;
			m_sql << "DELETE FROM objects WHERE id=" << do_id;

			if(dcc && storable)
			{
				m_log->spam() << "... object has stored field, also deleted." << endl;
				m_sql << "DELETE FROM fields_" << dcc->get_name() << " WHERE object_id=:id;", use(do_id);
			}

			push_id(do_id);
		}
		bool get_object(uint32_t do_id, DatabaseObject& dbo)
		{
			m_log->spam() << "Getting obj-" << do_id << " ..." << std::endl;

			// Get class from the objects table
			DCClass* dcc = get_class(do_id);
			if(!dcc)
			{
				return false; // Object does not exist
			}
			dbo.dc_id = dcc->get_number();

			bool stored = is_storable(dcc->get_number());
			if(stored)
			{
				get_fields_from_table(do_id, dcc, dbo.fields);
			}

			return true;
		}
		DCClass* get_class(uint32_t do_id)
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

			return g_dcf->get_class(dc_id);
		}
		void del_field(uint32_t do_id, DCField* field)
		{
		}
		void del_fields(uint32_t do_id, const std::vector<DCField*> &fields)
		{
		}
		void set_field(uint32_t do_id, DCField* field, const vector<uint8_t> &value)
		{
		}
		void set_fields(uint32_t do_id, const map<DCField*, vector<uint8_t>> &fields)
		{
		}
		bool set_field_if_empty(uint32_t do_id, DCField* field, vector<uint8_t> &value)
		{
			return false;
		}
		bool set_fields_if_empty(uint32_t do_id, map<DCField*, vector<uint8_t>> &values)
		{
			return false;
		}
		bool set_field_if_equals(uint32_t do_id, DCField* field, const vector<uint8_t> &equal, vector<uint8_t> &value)
		{
			return false;
		}
		bool set_fields_if_equals(uint32_t do_id, const map<DCField*, vector<uint8_t>> &equals, map<DCField*, vector<uint8_t>> &values)
		{
			return false;
		}

		bool get_field(uint32_t do_id, const DCField* field, vector<uint8_t> &value)
		{
			return false;
		}
		bool get_fields(uint32_t do_id,  const std::vector<DCField*> &fields, map<DCField*, vector<uint8_t>> &values)
		{
			return false;
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
			else if(m_backend == "sqlite")
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
			      "id INT NOT NULL PRIMARY KEY, hash INT NOT NULL, name VARCHAR(32) NOT NULL,"
			      "storable BOOLEAN NOT NULL);";//, CONSTRAINT check_class CHECK (id BETWEEN 0 AND "
			      //<< g_dcf->get_num_classes()-1 << "));";
		}

		void check_classes()
		{
			int dc_id;
			uint8_t storable;
			string dc_name;
			unsigned long dc_hash;

			// Prepare sql statements
			statement get_row_by_id = (m_sql.prepare << "SELECT hash, name FROM classes WHERE id=:id",
			                          into(dc_hash), into(dc_name), use(dc_id));
			statement insert_class = (m_sql.prepare << "INSERT INTO classes VALUES (:id,:hash,:name,:stored)",
			                          use(dc_id), use(dc_hash), use(dc_name), use(storable));

			// For each class, verify an entry exists and has the correct name and value
			for(dc_id = 0; dc_id < g_dcf->get_num_classes(); ++dc_id)
			{
				get_row_by_id.execute(true);
				if(m_sql.got_data())
				{
					check_class(dc_id, dc_name, dc_hash);
				}
				else
				{
					DCClass* dcc = g_dcf->get_class(dc_id);

					// Create fields table for the class
					storable = create_fields_table(dcc);

					// Create class row in classes table
					HashGenerator gen;
					dcc->generate_hash(gen);
					dc_hash = gen.get_hash();
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

			uint32_t id;

			// Get all ids from the database at once
			statement st = (m_sql.prepare << "SELECT id FROM objects;", into(id));
			st.execute();

			// Iterate through the result set, removing used ids from the free ids
			while(st.fetch())
			{
			    m_free_ids -= interval_t::closed(id, id);
			}
		}

		uint32_t pop_next_id()
		{
			// Check to make sure any free ids exist
			if(!m_free_ids.size())
			{
				return INVALID_DO_ID;
			}

			// Get next available id from m_free_ids set
			interval_t first = *m_free_ids.begin();
			uint32_t id = first.lower();
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

		void push_id(uint32_t id)
		{
		    m_free_ids += interval_t::closed(id, id);
		}
	private:
		uint32_t m_min_id, m_max_id;
		string m_backend, m_db_name;
		string m_sess_user, m_sess_passwd;
		session m_sql;
		set_t m_free_ids;
		LogCategory* m_log;

		void check_class(uint16_t id, string name, unsigned long hash)
		{
			DCClass* dcc = g_dcf->get_class(id);
			if(name != dcc->get_name())
			{
				// TODO: Try and update the database instead of exiting
				m_log->fatal() << "Class name '" << dcc->get_name() << "' from DCFile does not match"
				               " name '" << name << "' in database, for dc_id " << id << std::endl;
				m_log->fatal() << "Database must be rebuilt." << std::endl;
				exit(1);
			}

			HashGenerator gen;
			dcc->generate_hash(gen);
			if(hash != gen.get_hash())
			{
				// TODO: Try and update the database instead of exiting
				m_log->fatal() << "Class hash '" << gen.get_hash() << "' from DCFile does not match"
				               " hash '" << hash << "' in database, for dc_id " << id << std::endl;
				m_log->fatal() << "Database must be rebuilt." << std::endl;
				exit(1);
			}

			// TODO: Check class_fields table exists

		}

		// returns true if class has db fields
		bool create_fields_table(DCClass* dcc)
		{
			stringstream ss;
			ss << "CREATE TABLE IF NOT EXISTS fields_" << dcc->get_name()
			   << "(object_id INT NOT NULL PRIMARY KEY";

			int db_field_count = 0;
			for(int i = 0; i < dcc->get_num_inherited_fields(); ++i)
			{
				DCField* field = dcc->get_inherited_field(i);
				if(field->is_db() && !field->as_molecular_field())
				{
					db_field_count += 1;
					// TODO: Store SimpleParameters and fields with 1 SimpleParameter
					//       as a simpler type.
					// NOTE: This might be a lot easier if the DCParser was modified
					//       such that atomic fields containing only 1 DCSimpleParameter
					//       element are initialized as a DCSimpleField subclass of DCAtomicField.
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

		void get_fields_from_table(uint32_t id, DCClass* dcc, map<DCField*, vector<uint8_t>> &fields)
		{
			string value;
			indicator ind;
			for(int i = 0; i < dcc->get_num_inherited_fields(); ++i)
			{
				DCField* field = dcc->get_inherited_field(i);
				if(field->is_db())
				{
					m_sql << "SELECT " << field->get_name() << " FROM fields_" << dcc->get_name()
					      << " WHERE object_id=" << id << ";", into(value, ind);

					if(ind == i_ok)
					{
						string packed_data = field->parse_string(value);
						fields[field] = vector<uint8_t>(packed_data.begin(), packed_data.end());
					}
				}
			}
		}
		void set_fields_in_table(uint32_t id, DCClass* dcc, const map<DCField*, vector<uint8_t>> &fields)
		{
			string name, value;
			for(auto it = fields.begin(); it != fields.end(); ++it)
			{
				name = it->first->get_name();

				string packed_data(it->second.begin(), it->second.end());
				value = it->first->format_data(packed_data, false);

				m_sql << "UPDATE fields_" << dcc->get_name() << " SET " << name << "='" << value
				      << "' WHERE object_id=" << id << ";";
			}
		}
};

DBEngineCreator<SociSQLEngine> mysqlengine_creator("mysql");
DBEngineCreator<SociSQLEngine> postgresqlengine_creator("postgresql");
DBEngineCreator<SociSQLEngine> sqliteengine_creator("sqlite");
