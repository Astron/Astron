#pragma once
#include <boost/asio.hpp>
#include <functional>

// The HAProxyHandler class provides a single function, async_process, which
// reads HAProxy PROXY protocol v1/v2 connection information. This takes great
// care not to read any extra bytes after the proxy information, so that all
// application-layer bytes stay in the read buffer, just as if the socket was a
// direct connection. This allows the socket itself to be passed on to the
// next stage in the upper-level connection handling logic (e.g. SSL handshake
// or application-level network handlers.)

// Maximum size of the HAProxy header, per documentation:
#define HAPROXY_HEADER_MAX 107
// Minimum size of the HAProxy header:
#define HAPROXY_HEADER_MIN 16

using boost::asio::ip::tcp;

typedef std::function<void(const boost::system::error_code &, const tcp::endpoint &, const tcp::endpoint &)>
ProxyCallback;

class HAProxyHandler
{
  public:
    static void async_process(tcp::socket *socket, ProxyCallback &callback);

  private:
    HAProxyHandler(tcp::socket *socket, ProxyCallback &callback);
    ~HAProxyHandler();

    void begin();

    void handle_header(const boost::system::error_code &ec, size_t bytes_transferred);

    void handle_v2(const boost::system::error_code &ec, size_t bytes_transferred);
    void parse_v2();

    void handle_v1(const boost::system::error_code &ec, size_t bytes_transferred);
    void parse_v1();

    void finish(const boost::system::error_code &ec);

    tcp::socket *m_socket;
    ProxyCallback m_callback;

    tcp::endpoint m_remote;
    tcp::endpoint m_local;

    uint8_t m_header_buf[HAPROXY_HEADER_MAX + 1];
    size_t m_header_len = 0;

    uint8_t *m_body_buf = nullptr;
    size_t m_body_len = 0;
};
