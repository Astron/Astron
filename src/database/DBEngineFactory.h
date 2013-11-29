#pragma once
#include "IDatabaseEngine.h"
#include "core/config.h"
#include "core/types.h"
#include <string>
#include <cstdint>

// A BaseDBEngineFactoryItem is a common ancestor that all
//     DatabseEngine factory templates inherit from.
class BaseDBEngineFactoryItem
{
	public:
		virtual IDatabaseEngine* instantiate(DBEngineConfig config, doid_t min_id, doid_t max_id) = 0;
	protected:
		BaseDBEngineFactoryItem(const std::string &name);
};

// A DBEngineFactoryItem is the factory for a particular database backend.
// Each new role should declare a DBEngineFactoryItem<BackendClass>("BackendName");
template<class T>
class DBEngineFactoryItem : public BaseDBEngineFactoryItem
{
	public:
		DBEngineFactoryItem(const std::string& name) : BaseDBEngineFactoryItem(name)
		{
		}

		virtual IDatabaseEngine* instantiate(DBEngineConfig config, doid_t min_id, doid_t max_id)
		{
			return new T(config, min_id, max_id);
		}
};

// The DBEngineFactory is a singleton that instantiates DatabaseEngines from a backend's name.
class DBEngineFactory
{
	public:
		static DBEngineFactory singleton;

		// instantiate_engine creates a new IDatabaseEngine object of type 'engine_name'.
		IDatabaseEngine* instantiate_engine(const std::string &engine_name, DBEngineConfig config,
		                             doid_t min_id, doid_t max_id);


		// add_backend adds a factory for backend of type 'name'
		// It is called automatically when instantiating a new BaseDBEngineFactoryItem.
		void add_backend(const std::string &name, BaseDBEngineFactoryItem* factory);
	private:
		std::map<std::string, BaseDBEngineFactoryItem*> m_factories;
};
