#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "util/Datagram.h"

using boost::asio::ip::udp;

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
    udp::socket m_socket;
    udp::endpoint m_target;
    bool m_enabled;
};
