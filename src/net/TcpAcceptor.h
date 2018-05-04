#pragma once
#include "NetworkAcceptor.h"
#include <functional>

typedef std::function<void(const std::shared_ptr<uvw::TcpHandle>&, const uvw::Addr& remote, const uvw::Addr& local, const bool haproxy_mode)> TcpAcceptorCallback;

class TcpAcceptor : public NetworkAcceptor
{
  public:
    TcpAcceptor(TcpAcceptorCallback &callback, AcceptorErrorCallback &err_callback);
    virtual ~TcpAcceptor() {}

  private:
    TcpAcceptorCallback m_callback;

    virtual void start_accept();
    void handle_accept(const std::shared_ptr<uvw::TcpHandle>& socket);
    void handle_endpoints(const std::shared_ptr<uvw::TcpHandle>& socket, const uvw::Addr& remote, const uvw::Addr& local);
};
