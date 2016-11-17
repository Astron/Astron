#pragma once
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class NetworkAcceptor
{
  public:
    virtual ~NetworkAcceptor() {}

    // Parses the string "address" and binds to it. If no port is specified
    // as part of the address, it will use default_port.
    boost::system::error_code bind(const std::string &address, unsigned int default_port);

    void start();
    void stop();

    inline void set_haproxy_mode(bool haproxy_mode)
    {
        m_haproxy_mode = haproxy_mode;
    }

  protected:
    boost::asio::io_service &m_io_service;
    tcp::acceptor m_acceptor;
    bool m_started;
    bool m_haproxy_mode = false;

    NetworkAcceptor(boost::asio::io_service&);

    virtual void start_accept() = 0;
};
