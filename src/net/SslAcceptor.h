#pragma once
#include "NetworkAcceptor.h"
#include <functional>
#include <boost/asio/ssl.hpp>
namespace ssl = boost::asio::ssl;

typedef std::function<void(ssl::stream<tcp::socket>*)> SslAcceptorCallback;

class SslAcceptor : public NetworkAcceptor
{
  public:
    SslAcceptor(boost::asio::io_service &io_service, ssl::context& ctx,
                SslAcceptorCallback &callback);
    virtual ~SslAcceptor() {}

  private:
    ssl::context& m_context;
    SslAcceptorCallback m_callback;

    virtual void start_accept();
    void handle_accept(ssl::stream<tcp::socket> *socket, const boost::system::error_code &ec);
    void handle_handshake(ssl::stream<tcp::socket> *socket, const boost::system::error_code &ec);
};
