#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "core/global.h"

#include <soci.h>

using namespace std;
using namespace soci;

static ConfigVariable<std::string> engine_type("type", "null");
static ConfigVariable<std::string> database_name("database", "null");
static ConfigVariable<std::string> session_user("username", "null");
static ConfigVariable<std::string> session_passwd("password", "null");

class SociSQLEngine : public IDatabaseEngine
{
	public:
		SociSQLEngine(DBEngineConfig dbeconfig, uint32_t min_id, uint32_t max_id) :
			(dbeconfig, min_id, max_id), m_backend(engine_type.get_rval()),
			m_db_name(database_name.get_rval()), m_sess_user(session_user.get_rval()),
			m_sess_passwd(session_passwd.get_rval())
		{
			stringstream log_name;
			log_name << "Database-" << m_backend << "(Range: [" << min_id << ", " << max_id << "])";
			m_log = new LogCategory(m_backend, log_name.string);

			connect();
			check_tables();
			check_classes();
		}
	protected:
		void connect()
		{
			stringstream connstring;
			if(backend == "postgresql")
			{
				connstring << "dbname=" << m_db_name;
				m_sql.open(postgresql, connstring.str());
			}
			else if(backend == "mysql")
			{
				connstring << "db=" << m_db_name << " "
				           << "user=" << m_sess_user << " "
				           << "pass='" << m_sess_passwd << "'";
				m_sql.open(mysql, connstring.str());
			}
			else if(backend == "sqlite")
			{
				m_sql.open(sqlite3, m_db_name);
			}
		}

		void check_tables()
		{
			m_sql << "CREATE TABLE IF NOT EXISTS objects ("
			      "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, class_id INT NOT NULL,"
			      "CONSTRAINT check_object CHECK (id BETWEEN " << min_id << "AND" << max_id "));";
			m_sql << "CREATE TABLE IF NOT EXISTS classes ("
			      "id INT NOT NULL PRIMARY KEY, hash INT NOT NULL, name VARCHAR(32) NOT NULL,"
			      "CONSTRAINT check_class CHECK (id BETWEEN 0 AND " << g_dcf->get_num_classes()-1 << "));";
		}

		void check_classes()
		{
			int dc_id;
			string dc_name;
			unsigned long dc_hash;

			// Prepare sql statements
			statement get_row_by_id = (m_sql.prepare << "SELECT hash, name FROM classes WHERE id=:id",
			                          into(dc_hash), into(dc_name), use(dc_id));
			statement insert_class = (m_sql.prepare << "INSERT INTO classes VALUES (:id,:hash,:name)",
			                          use(dc_id), use(dc_hash), use(dc_name));

			// For each class, verify an entry exists and has the correct name and value
			for(dc_id = 0; dc_id < g_dcf->get_num_classes(); ++dc_id)
			{
				get_row_by_id.execute(true);
				if(sql.got_data())
				{
					check_class(dc_id, dc_name, dc_hash);
				}
				else
				{
					DCClass* dcc = g_dcf->get_class(dc_id);

					// Create fields table for the class
					if(create_fields_table(dcc))
					{
						// Create class row in classes table
						HashGenerator gen();
						dcc->generate_hash(gen);
						dc_hash = gen.get_hash();
						dc_name = dcc->get_name();
						insert_class.execute(true);
					}
				}
			}
		}
	private:
		string m_backend, m_db_name;
		string m_sess_user, m_sess_passwd;
		session m_sql;
		LogCategory* m_log;

		void check_class(uint16_t id, string name, unsigned long hash)
		{
			DCClass* dcc = g_dcf->get_class(dc_id);
			if(dc_name != dcc->get_name())
			{
				// TODO: Try and update the database instead of exiting
				m_log->fatal() << "Class name '" << dcc->get_name() << "' from DCFile does not match"
				               " name '" << dc_name << "' in database, for dc_id " << dc_id << std::endl;
				m_log->fatal() << "Database must be rebuilt." << std::endl;
				exit(1);
			}

			HashGenerator gen();
			dcc->generate_hash(gen);
			if(dc_hash != gen.get_hash())
			{
				// TODO: Try and update the database instead of exiting
				m_log->fatal() << "Class hash '" << gen.get_hash() << "' from DCFile does not match"
				               " hash '" << dc_hash << "' in database, for dc_id " << dc_id << std::endl;
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
					ss << "," << field->get_name() << " "; // column name
					switch(field->get_pack_type())
					{
						case PT_int, PT_uint, PT_int64, PT_uint64:
						{
							ss << "INT"; //column type
						}
						break;
						case PT_string:
						{
							// TODO: See if its possible to get the parameter range limits,
							//       then use VARCHAR(n) for smaller range limits.
							ss << "TEXT"; //column type
						}
						break;
						default:
						{
							// TODO: See if its possible to get the parameter range limits,
							//       then use VARBINARY(n) for smaller range limits.
							if(backend == "postgresql")
							{
								ss << "BYTEA";
							}
							else
							{
								ss << "BLOB";
							}
						}
					}
				}
			}

			if(db_field_count > 0)
			{
				m_sql << ss.str();
				return true;
			}

			return false;
		}
};

DBEngineCreator<SociEngine> mysqlengine_creator("mysql");
DBEngineCreator<SociEngine> postgresqlengine_creator("postgresql");
DBEngineCreator<SociEngine> sqliteengine_creator("sqlite");
