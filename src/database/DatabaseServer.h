#pragma once
#include <unordered_map>

#include "core/Role.h"
#include "core/RoleFactory.h"
#include "DatabaseBackend.h"
#include "DBOperation.h"
#include "DBOperationQueue.h"

extern RoleConfigGroup dbserver_config;

class DatabaseServer : public Role
{
  public:
    DatabaseServer(RoleConfig);

    virtual void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

  private:
    void handle_operation(DBOperation *op);
    void clear_operation(const DBOperation *op);
    std::unordered_map<doid_t, DBOperationQueue> m_queues;
    std::recursive_mutex m_lock;

    DatabaseBackend *m_db_backend;
    LogCategory *m_log;

    channel_t m_control_channel;
    doid_t m_min_id, m_max_id;
    bool m_broadcast;

    friend class DBOperation;
    friend class DBOperationCreate;
    friend class DBOperationDelete;
    friend class DBOperationGet;
    friend class DBOperationSet;
    friend class DBOperationUpdate;

    friend class DBOperationQueue;
};
