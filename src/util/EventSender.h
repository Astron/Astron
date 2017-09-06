#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <uvw.hpp>

#include "util/Datagram.h"

// This is a convenience class for building up MessagePack logs.
class LoggedEvent
{
  public:
    LoggedEvent();
    LoggedEvent(const std::string &type);
    LoggedEvent(const std::string &type, const std::string &sender);

    void add(const std::string &key, const std::string &value);

    DatagramHandle make_datagram() const;

  private:
    std::vector<std::pair<std::string, std::string> > m_kv;
    std::unordered_map<std::string, size_t> m_keys;
};

class EventSender
{
  public:
    EventSender();

    void init(const std::string &target);
    void send(DatagramHandle dg);
    inline void send(const LoggedEvent &event)
    {
        send(event.make_datagram());
    }
  private:
    LogCategory m_log;
    uvw::Addr m_target;
    std::shared_ptr<uvw::UDPHandle> m_socket;
    bool m_enabled;
};
