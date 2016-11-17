#include "SslAcceptor.h"
#include "HAProxyHandler.h"
#include <boost/bind.hpp>

SslAcceptor::SslAcceptor(boost::asio::io_service &io_service, ssl::context& ctx,
                         SslAcceptorCallback &callback) :
    NetworkAcceptor(io_service),
    m_context(ctx),
    m_callback(callback)
{
}

void SslAcceptor::start_accept()
{
    ssl::stream<tcp::socket> *socket = new ssl::stream<tcp::socket>(m_io_service, m_context);
    m_acceptor.async_accept(socket->next_layer(), boost::bind(&SslAcceptor::handle_accept, this,
                            socket, boost::asio::placeholders::error));
}

void SslAcceptor::handle_accept(ssl::stream<tcp::socket> *socket,
                                const boost::system::error_code &ec)
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
        ProxyCallback callback = std::bind(&SslAcceptor::handle_endpoints,
                                           this, socket,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3);
        HAProxyHandler::async_process(&socket->next_layer(), callback);
    } else {
        boost::system::error_code endpoint_ec;
        tcp::endpoint remote = socket->next_layer().remote_endpoint(endpoint_ec);
        tcp::endpoint local = socket->next_layer().local_endpoint(endpoint_ec);
        handle_endpoints(socket, endpoint_ec, remote, local);
    }
}

void SslAcceptor::handle_endpoints(ssl::stream<tcp::socket> *socket,
                                   const boost::system::error_code &ec,
                                   const tcp::endpoint &remote,
                                   const tcp::endpoint &local)
{
    if(ec) {
        // The accept failed for some reason.
        delete socket;
        return;
    }

    // Dispatch a handshake (and appropriate timeout)
    auto timeout = std::make_shared<Timeout>(
                       m_handshake_timeout,
                       std::bind(&SslAcceptor::handle_timeout, this, socket));
    socket->async_handshake(ssl::stream<tcp::socket>::server,
                            boost::bind(&SslAcceptor::handle_handshake, this,
                                        socket, timeout,
                                        boost::asio::placeholders::error,
                                        remote, local));
    if(m_handshake_timeout > 0) {
        timeout->start();
    }
}

void SslAcceptor::handle_handshake(ssl::stream<tcp::socket> *socket,
                                   std::shared_ptr<Timeout> timeout,
                                   const boost::system::error_code &ec,
                                   const tcp::endpoint &remote,
                                   const tcp::endpoint &local)
{
    if(!timeout->cancel()) {
        // We failed to cancel the timeout! That means it ran and got to our
        // socket first. Oh well.
        delete socket;
        return;
    }

    if(!m_started) {
        // We were turned off sometime before this operation completed; ignore.
        delete socket;
        return;
    }

    if(ec) {
        // The handshake failed for some reason. Free the socket and try again:
        delete socket;
        return;
    }

    // Inform the callback:
    m_callback(socket, remote, local);
}

void SslAcceptor::handle_timeout(ssl::stream<tcp::socket> *socket)
{
    // Handshake didn't complete in time. Kick out the client:
    socket->next_layer().close();
    // Socket NOT deleted here. handle_handshake above will run with an error
    // code.
}
