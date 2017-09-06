#pragma once
#include <fstream>
#include <memory>

#include <uvw.hpp>

#include "core/global.h"
#include "core/Role.h"
#include "util/Datagram.h"

// An EventLogger is a role in the daemon that opens up a local socket and reads UDP packets from
// that socket.  Received UDP packets will be logged as configured by the daemon config file.
class EventLogger final : public Role
{
  public:
    EventLogger(RoleConfig roleconfig);

    void handle_datagram(DatagramHandle, DatagramIterator&) { } // Doesn't take DGs.

  private:
    LogCategory m_log;
    std::shared_ptr<uvw::UDPHandle> m_socket;
    std::string m_file_format;
    std::unique_ptr<std::ofstream> m_file;

    void bind(const std::string &addr);
    void open_log();
    void cycle_log();
    void start_receive();
    void handle_receive(const uvw::UDPDataEvent& event);
    void handle_error(const uvw::ErrorEvent& error);
    void process_packet(DatagramHandle dg);
};
