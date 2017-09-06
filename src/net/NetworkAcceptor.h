#pragma once

#include <uvw.hpp>

class NetworkAcceptor
{
  public:
    virtual ~NetworkAcceptor() {}

    // Parses the string "address" and binds to it. If no port is specified
    // as part of the address, it will use default_port.
    bool bind(const std::string &address, unsigned int default_port);

    void start();
    void stop();

    inline void set_haproxy_mode(bool haproxy_mode)
    {
        m_haproxy_mode = haproxy_mode;
    }

  protected:
    void error_callback(const uvw::ErrorEvent& err);

    std::shared_ptr<uvw::TcpHandle> m_acceptor;
    bool m_started;
    bool m_haproxy_mode = false;

    NetworkAcceptor();

    virtual void start_accept() = 0;
};
