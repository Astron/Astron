#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

#include "util/Datagram.h"
#include "util/DatagramIterator.h"
#include "core/logger.h"
#include "core/global.h"

#include <fstream>
#include <list>

ConfigVariable<std::string> foldername("foldername", "yaml_db");
LogCategory fsdb_log("YAML-DB", "YAML Database Engine");

class YAMLEngine : public IDatabaseEngine
{
private:
	unsigned int m_next_id;
	std::list<unsigned int> m_free_ids;
	std::string m_foldername;
public:
	YAMLEngine(DBEngineConfig dbeconfig, unsigned int start_id) :
		IDatabaseEngine(dbeconfig, start_id),
		m_next_id(start_id),
		m_free_ids()
		m_foldername(foldername.get_rval(m_dbeconfig))
	{
	}

	unsigned int get_next_id()
	{

	}

	bool create_object(const DatabaseObject &dbo)
	{
		if(!m_free_ids.empty())
		{
			return *m_free_ids.begin();
		}
		return m_next_id;
	}

	bool get_object(DatabaseObject &dbo)
	{

	}

	void delete_object(unsigned int do_id)
	{

	}

};

DBEngineCreator<FSDBEngine> yamlengine_creator("yaml");