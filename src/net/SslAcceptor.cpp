#include "SslAcceptor.h"
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

    if(ec.value() != 0) {
        // The accept failed for some reason. Free the socket and try again:
        delete socket;
        start_accept();
        return;
    }

    // Dispatch a handshake (and appropriate timeout)
    auto timeout = std::make_shared<Timeout>(
            m_handshake_timeout,
            std::bind(&SslAcceptor::handle_timeout, this, socket));
    socket->async_handshake(ssl::stream<tcp::socket>::server,
                            boost::bind(&SslAcceptor::handle_handshake, this,
                                        socket, timeout,
                                        boost::asio::placeholders::error));
    if(m_handshake_timeout > 0) {
        timeout->start();
    }

    // Start accepting again:
    start_accept();
}

void SslAcceptor::handle_handshake(ssl::stream<tcp::socket> *socket,
                                   std::shared_ptr<Timeout> timeout,
                                   const boost::system::error_code &ec)
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

    if(ec.value() != 0) {
        // The handshake failed for some reason. Free the socket and try again:
        delete socket;
        return;
    }

    // Inform the callback:
    m_callback(socket);
}

void SslAcceptor::handle_timeout(ssl::stream<tcp::socket> *socket)
{
    // Handshake didn't complete in time. Kick out the client:
    socket->next_layer().close();
    // Socket NOT deleted here. handle_handshake above will run with an error
    // code.
}
