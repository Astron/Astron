#pragma once
#include <memory>
#include <unordered_map>
#include "core/Role.h"
#include "core/RoleFactory.h"

class DistributedObject;

class StateServer : public Role
{
    friend class DistributedObject;

  public:
    StateServer(RoleConfig roleconfig);

    virtual void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

  protected:
    std::unique_ptr<LogCategory> m_log;
    std::unordered_map<doid_t, DistributedObject*> m_objs;

  private:
    void handle_generate(DatagramIterator &dgi, bool has_other);
    void handle_delete_ai(DatagramIterator &dgi, channel_t sender);
};
