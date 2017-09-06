#pragma once

#include "net/SocketWrapper.h"

#include <functional>
#include <uvw.hpp>

typedef std::function<void(SocketPtr, const uvw::ErrorEvent &)> ConnectCallback;

class NetworkConnector
{
  public:
    NetworkConnector(ConnectCallback callback);
    ~NetworkConnector();

    // Parses the string "address" and connects to it. If no port is specified
    // as part of the address, it will use default_port.
    // The return value will either be a freshly-allocated socket, or nullptr.
    // If the return value is nullptr, the error code will be set to indicate
    // the reason that the connect failed.
    void connect(const std::string &address, unsigned int default_port);

  private:
    void do_connect(const std::string &address, uint16_t port);

    ConnectCallback m_callback;

    std::shared_ptr<uvw::TcpHandle> m_socket;
};
