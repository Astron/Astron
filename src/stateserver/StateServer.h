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

    // Our SS metrics here:
    prometheus::Family<prometheus::Gauge>* m_ss_objs_builder = nullptr;
    std::unordered_map<uint16_t, prometheus::Gauge*> m_ss_objs_gauges;
    prometheus::Family<prometheus::Histogram>* m_ss_obj_size_builder = nullptr;
    std::unordered_map<uint16_t, prometheus::Histogram*> m_ss_obj_size_hist;
    prometheus::Family<prometheus::Counter> *m_ss_obj_creation_builder = nullptr;
    prometheus::Counter *m_ss_obj_creation_ctr = nullptr;
    prometheus::Family<prometheus::Counter> *m_ss_obj_deletion_builder = nullptr;
    prometheus::Counter *m_ss_obj_deletion_cnt = nullptr;
    prometheus::Family<prometheus::Counter> *m_ss_obj_change_builder = nullptr;
    prometheus::Counter *m_ss_obj_change_cnt = nullptr;
    prometheus::Family<prometheus::Counter> *m_ss_obj_query_builder = nullptr;
    prometheus::Counter *m_ss_obj_query_cnt = nullptr;
  private:
    void init_metrics();
    void handle_generate(DatagramIterator &dgi, bool has_other);
    void handle_delete_ai(DatagramIterator &dgi, channel_t sender);
};
