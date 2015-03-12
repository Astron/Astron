#include "TcpAcceptor.h"
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

    if(ec.value() != 0) {
        // The accept failed for some reason. Free the socket and try again:
        delete socket;
        start_accept();
        return;
    }

    // Inform the callback:
    m_callback(socket);

    // Start accepting again:
    start_accept();
}
