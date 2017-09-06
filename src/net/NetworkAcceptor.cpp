#include "NetworkAcceptor.h"
#include "address_utils.h"

NetworkAcceptor::NetworkAcceptor() :
    m_started(false),
    m_loop(nullptr),
    m_acceptor(nullptr)
{
    m_loop = uvw::Loop::getDefault();
}

void NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    m_acceptor = m_loop->resource<uvw::TcpHandle>();

    std::vector<uvw::Addr> addresses = resolve_address(address, default_port, m_loop);

    for (uvw::Addr& addr : addresses) {
	    m_acceptor->bind(addr);
    }
}

void NetworkAcceptor::start()
{
    if(m_started) {
        // Already started, start() was called twice!
        return;
    }

    m_started = true;

    // Sets up the handlers for accepting peers.
    start_accept();

    // Start listening for inbound connections.
    m_acceptor->listen();

    m_loop->run();
}

void NetworkAcceptor::stop()
{
    if(!m_started) {
        // Already stopped, stop() was called twice!
        return;
    }

    m_started = false;

    m_acceptor->close();
}
