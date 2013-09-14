#pragma once
#include "core/config.h"
#include "dcparser/dcField.h"

class IDatabaseEngine
{
	public:
		IDatabaseEngine(DBEngineConfig dbeconfig, unsigned int start_do_id);

		virtual unsigned int get_next_id() = 0;
		virtual bool create_object(unsigned int do_id, const std::map<DCField*, std::string> &fields);
	private:
		DBEngineConfig m_dbeconfig;
};