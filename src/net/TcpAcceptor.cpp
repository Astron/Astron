#include "TcpAcceptor.h"
#include "HAProxyHandler.h"

TcpAcceptor::TcpAcceptor(TcpAcceptorCallback callback) :
    NetworkAcceptor(),
    m_callback(callback)
{
}

void TcpAcceptor::start_accept()
{
    m_acceptor->on<uvw::ListenEvent>([this](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        auto client = srv.loop().resource<uvw::TcpHandle>();
        srv.accept(*client);
        handle_accept(client);
    });

    m_acceptor->listen();
}

void TcpAcceptor::handle_accept(std::shared_ptr<uvw::TcpHandle> socket)
{
    if(!m_started) {
        // We were turned off sometime before this operation completed; ignore.
        socket->close();
        return;
    }

    auto socket_ptr = std::make_shared<SocketWrapper>(socket);
    socket_ptr->initialize();

    if(m_haproxy_mode) {
        HAProxyHandler::async_process(socket_ptr, [this, socket_ptr](
                                  const uvw::ErrorEvent& error,
                                  const uvw::Addr &remote,
                                  const uvw::Addr &local) {
            if (error)
            {
                socket_ptr->clear();
                return;
            }

            this->handle_endpoints(socket_ptr, remote, local);
        });
    } else {
        auto remote = socket->peer();
        auto local = socket->sock();
        handle_endpoints(socket_ptr, remote, local);
    }
}

void TcpAcceptor::handle_endpoints(SocketPtr socket,
                                   const uvw::Addr &remote,
                                   const uvw::Addr &local)
{
    // Inform the callback:
    m_callback(socket, remote, local);
}
