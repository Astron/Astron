#pragma once
#include "NetworkAcceptor.h"
#include "util/Timeout.h"
#include <functional>
#include <boost/asio/ssl.hpp>
namespace ssl = boost::asio::ssl;

typedef std::function<void(ssl::stream<tcp::socket>*, tcp::endpoint, tcp::endpoint)>
SslAcceptorCallback;

class SslAcceptor : public NetworkAcceptor
{
  public:
    SslAcceptor(boost::asio::io_service &io_service, ssl::context& ctx,
                SslAcceptorCallback &callback);

    inline void set_handshake_timeout(int milliseconds)
    {
        m_handshake_timeout = milliseconds;
    }

  private:
    ssl::context& m_context;
    SslAcceptorCallback m_callback;

    int m_handshake_timeout = 0;

    virtual void start_accept();
    void handle_accept(ssl::stream<tcp::socket> *socket, const boost::system::error_code &ec);
    void handle_endpoints(ssl::stream<tcp::socket> *socket,
                          const boost::system::error_code &ec,
                          const boost::asio::ip::tcp::endpoint &remote,
                          const boost::asio::ip::tcp::endpoint &local);
    void handle_handshake(ssl::stream<tcp::socket> *socket, std::shared_ptr<Timeout> timeout,
                          const boost::system::error_code &ec,
                          const boost::asio::ip::tcp::endpoint &remote,
                          const boost::asio::ip::tcp::endpoint &local);
    void handle_timeout(ssl::stream<tcp::socket> *socket);
};
