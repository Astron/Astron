#include "IDatabaseEngine.h"
#include "DBEngineFactory.h"

class FSDBEngine : public IDatabaseEngine
{
	private:
		unsigned int m_next_id;
	public:
		FSDBEngine(DBEngineConfig dbeconfig, unsigned int start_id) : IDatabaseEngine(dbeconfig, start_id),
			m_next_id(start_id)
		{
		}

		virtual unsigned int get_next_id()
		{
			return m_next_id;
		}

		virtual bool create_object(unsigned int do_id, const std::map<DCField*, std::string> &fields)
		{
			if(do_id != m_next_id)
			{
				return false;
			}
			
			return false;//TODO: implement me so I can return true;
		}
};

DBEngineCreator<FSDBEngine> fsdbengine_creator("filesystem");