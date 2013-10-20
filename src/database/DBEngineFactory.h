#pragma once
#include <string>
#include <cstdint>
#include "core/config.h"

class IDatabaseEngine;

class BaseDBEngineCreator
{
	public:
		virtual IDatabaseEngine* instantiate(DBEngineConfig config, uint32_t min_id, uint32_t max_id) = 0;
};

class DBEngineFactory
{
	public:
		static DBEngineFactory singleton;
		IDatabaseEngine* instantiate(const std::string &engine_name, DBEngineConfig config, uint32_t min_id,
		                             uint32_t max_id);
	private:
		DBEngineFactory();
		std::map<std::string, BaseDBEngineCreator*> m_creators;

		template<class T>
		friend class DBEngineCreator;
		void add_creator(const std::string &name, BaseDBEngineCreator* creator);
};

template<class T>
class DBEngineCreator : public BaseDBEngineCreator
{
	public:
		DBEngineCreator(const std::string& name)
		{
			DBEngineFactory::singleton.add_creator(name, this);
		}

		virtual IDatabaseEngine* instantiate(DBEngineConfig config, uint32_t min_id, uint32_t max_id)
		{
			return new T(config, min_id, max_id);
		}
};