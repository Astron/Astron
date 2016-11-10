#include "TcpAcceptor.h"
#include "HAProxyHandler.h"
#include <boost/bind.hpp>

TcpAcceptor::TcpAcceptor(boost::asio::io_service &io_service,
                         TcpAcceptorCallback &callback) :
    NetworkAcceptor(io_service),
    m_callback(callback)
{
}

void TcpAcceptor::start_accept()
{
    tcp::socket *socket = new tcp::socket(m_io_service);
    m_acceptor.async_accept(*socket,
                            boost::bind(&TcpAcceptor::handle_accept, this,
                                        socket, boost::asio::placeholders::error));
}

void TcpAcceptor::handle_accept(tcp::socket *socket, const boost::system::error_code &ec)
{
    if(!m_started) {
        // We were turned off sometime before this operation completed; ignore.
        delete socket;
        return;
    }

    // Start accepting another connection now:
    start_accept();

    if(ec) {
        // The accept failed for some reason.
        delete socket;
        return;
    }

    if(m_haproxy_mode) {
        ProxyCallback callback = std::bind(&TcpAcceptor::handle_endpoints,
                                           this, socket,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3);
        HAProxyHandler::async_process(socket, callback);
    } else {
        boost::system::error_code endpoint_ec;
        tcp::endpoint remote = socket->remote_endpoint(endpoint_ec);
        tcp::endpoint local = socket->local_endpoint(endpoint_ec);
        handle_endpoints(socket, endpoint_ec, remote, local);
    }
}

void TcpAcceptor::handle_endpoints(tcp::socket *socket, const boost::system::error_code &ec,
                                   const tcp::endpoint &remote,
                                   const tcp::endpoint &local)
{
    if(ec) {
        // The accept failed for some reason.
        delete socket;
        return;
    }

    // Inform the callback:
    m_callback(socket, remote, local);
}
