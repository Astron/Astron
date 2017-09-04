#include "TcpAcceptor.h"
#include "HAProxyHandler.h"
#include <boost/bind.hpp>

TcpAcceptor::TcpAcceptor(TcpAcceptorCallback &callback) :
    NetworkAcceptor(),
    m_callback(callback)
{
}

void TcpAcceptor::start_accept()
{
    m_acceptor->on<uvw::ListenEvent>([acceptor = this](const uvw::ListenEvent &, uvw::TcpHandle &srv) {
        std::shared_ptr<uvw::TcpHandle> client = srv.loop().resource<uvw::TcpHandle>();

        client->on<uvw::CloseEvent>([ptr = srv.shared_from_this()](const uvw::CloseEvent &, uvw::TcpHandle &) { ptr->close(); });
        client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TcpHandle &client) { client.close(); });

        srv.accept(*client);
        acceptor->handle_accept(client);
    });
}

void TcpAcceptor::handle_accept(std::shared_ptr<uvw::TcpHandle>& socket)
{
    if(!m_started) {
        // We were turned off sometime before this operation completed; ignore.
        socket->close();
        return;
    }

    if(m_haproxy_mode) {
        ProxyCallback callback = std::bind(&TcpAcceptor::handle_endpoints,
                                           this, socket,
                                           std::placeholders::_1,
                                           std::placeholders::_2);
        //HAProxyHandler::async_process(socket, callback);
    }
    else {
        uvw::Addr remote_endpoint = socket->peer();
        uvw::Addr local_endpoint = socket->sock();
        handle_endpoints(socket, remote, local);
    }
}

void TcpAcceptor::handle_endpoints(std::shared_ptr<uvw::TcpHandle>& socket, const uvw::Addr &remote, const uvw::Addr &local)
{
    // Inform the callback:
    m_callback(socket, remote, local);
}
