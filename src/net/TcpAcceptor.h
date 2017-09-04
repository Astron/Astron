#pragma once
#include "NetworkAcceptor.h"
#include <functional>

typedef std::function<void(std::shared_ptr<uvw::TcpHandle>&)> TcpAcceptorCallback;

class TcpAcceptor : public NetworkAcceptor
{
  public:
    TcpAcceptor(TcpAcceptorCallback &callback);
    virtual ~TcpAcceptor() {}

  private:
    TcpAcceptorCallback m_callback;

    virtual void start_accept();
    void handle_accept(std::shared_ptr<uvw::TcpHandle>& socket);
    void handle_endpoints(std::shared_ptr<uvw::TcpHandle>& socket, uvw::Addr& remote, uvw::Addr& local);
};
