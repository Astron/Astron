#pragma once
#include "NetworkAcceptor.h"
#include <functional>

typedef std::function<void(tcp::socket*, tcp::endpoint, tcp::endpoint)> TcpAcceptorCallback;

class TcpAcceptor : public NetworkAcceptor
{
  public:
    TcpAcceptor(boost::asio::io_service &io_service,
                TcpAcceptorCallback &callback);
    virtual ~TcpAcceptor() {}

  private:
    TcpAcceptorCallback m_callback;

    virtual void start_accept();
    void handle_accept(tcp::socket *socket, const boost::system::error_code &ec);
    void handle_endpoints(tcp::socket *socket, const boost::system::error_code &ec,
                          const tcp::endpoint &remote, const tcp::endpoint &local);
};
