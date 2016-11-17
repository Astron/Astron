#pragma once
#include <fstream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "core/Role.h"
#include "util/Datagram.h"

using boost::asio::ip::udp;

// There will typically only be one Event Logger, so we can afford to make the
// receive buffer pretty big.
#define EVENTLOG_BUFSIZE 8192

// An EventLogger is a role in the daemon that opens up a local socket and reads UDP packets from
// that socket.  Received UDP packets will be logged as configured by the daemon config file.
class EventLogger final : public Role
{
  public:
    EventLogger(RoleConfig roleconfig);

    void handle_datagram(DatagramHandle, DatagramIterator&) { } // Doesn't take DGs.

  private:
    LogCategory m_log;
    std::unique_ptr<udp::socket> m_socket;
    udp::endpoint m_remote;
    std::string m_file_format;
    std::unique_ptr<std::ofstream> m_file;
    uint8_t m_buffer[EVENTLOG_BUFSIZE];

    void bind(const std::string &addr);
    void open_log();
    void cycle_log();
    void start_receive();
    void handle_receive(const boost::system::error_code &error, std::size_t bytes);
    void process_packet(DatagramHandle dg);
};
