#pragma once
#include "NetworkAcceptor.h"
#include "SocketWrapper.h"

#include <functional>
#include <uvw.hpp>

typedef std::function<void(SocketPtr, uvw::Addr, uvw::Addr)> TcpAcceptorCallback;

class TcpAcceptor : public NetworkAcceptor
{
  public:
    TcpAcceptor(TcpAcceptorCallback callback);
    virtual ~TcpAcceptor() {}

  private:
    TcpAcceptorCallback m_callback;

    virtual void start_accept();
    void handle_accept(std::shared_ptr<uvw::TcpHandle> socket);
    void handle_endpoints(SocketPtr socket,
                          const uvw::Addr &remote,
                          const uvw::Addr &local);
};
