#pragma once
#include <deps/uvw/uvw.hpp>

typedef std::function<void(const std::shared_ptr<uvw::TcpHandle> &)> ConnectCallback;

class NetworkConnector : std::enable_shared_from_this<NetworkConnector>
{
  public:
    // Parses the string "address" and connects to it. If no port is specified
    // as part of the address, it will use default_port.
    // The provided callback will be invoked with the created socket post-connection.

    NetworkConnector(const std::shared_ptr<uvw::Loop> &loop);
    void connect(const std::string &address, unsigned int default_port,
                         ConnectCallback callback);
  private:
    std::shared_ptr<uvw::TcpHandle> m_socket;
    std::shared_ptr<uvw::Loop> m_loop;
    ConnectCallback connect_callback;

    void do_connect(const std::string &address, uint16_t port);
};
