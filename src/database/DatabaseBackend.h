#pragma once
#include "DBOperation.h"
#include "config/ConfigVariable.h"

extern KeyedConfigGroup db_backend_config;
extern ConfigVariable<std::string> db_backend_type;

class DatabaseBackend
{
  public:
    DatabaseBackend(ConfigNode dbeconfig, doid_t min_id, doid_t max_id) :
        m_config(dbeconfig), m_min_id(min_id), m_max_id(max_id) {}

    // This function is used to submit a DBOperation to the database backend
    // for execution. The operation need not be completed before the function
    // returns; it may even complete in a separate thread, since the DBOperation
    // class is thread-safe.
    //
    // Note: The backend must also be thread-safe as it is possible for two
    // or more invocations of submit() to occur in parallel. In addition,
    // the backend must be able to handle two concurrent operations on the
    // same database object.
    virtual void submit(DBOperation *operation) = 0;

  protected:
    ConfigNode m_config;
    doid_t m_min_id;
    doid_t m_max_id;
};
