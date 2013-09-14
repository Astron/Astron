#pragma once
#include <string>
#include "core/config.h"

class IDatabaseEngine;

class DBEngineFactory
{
	public:
		static DBEngineFactory singleton;
		IDatabaseEngine* instantiate(const std::string &engine_name, DBEngineConfig config, unsigned int start_do_id);
		

	private:
		DBEngineFactory();
		~DBEngineFactory();
};