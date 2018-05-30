#pragma once
#include "deps/uvw/uvw.hpp"

typedef std::function<void(const std::shared_ptr<uvw::TcpHandle> &)> ConnectCallback;
typedef std::function<void(const uvw::ErrorEvent& evt)> ConnectErrorCallback;

class NetworkConnector : public std::enable_shared_from_this<NetworkConnector>
{
  public:
    // Parses the string "address" and connects to it. If no port is specified
    // as part of the address, it will use default_port.
    // The provided callback will be invoked with the created socket post-connection.

    NetworkConnector(const std::shared_ptr<uvw::Loop> &loop);
    void destroy();
    void connect(const std::string &address, unsigned int default_port,
                 ConnectCallback callback, ConnectErrorCallback err_callback);
  private:
    std::shared_ptr<uvw::TcpHandle> m_socket;
    std::shared_ptr<uvw::Loop> m_loop;
    ConnectCallback m_connect_callback;
    ConnectErrorCallback m_err_callback;

    void do_connect(const std::string &address, uint16_t port);
};
