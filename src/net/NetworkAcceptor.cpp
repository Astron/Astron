#include "NetworkAcceptor.h"
#include "address_utils.h"


NetworkAcceptor::NetworkAcceptor() :
    m_loop(uvw::Loop::getDefault()),
    m_acceptor(nullptr),
    m_started(false)
{
}

void NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    m_acceptor = m_loop->resource<uvw::TcpHandle>();
    m_acceptor->simultaneousAccepts(true);

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

    // Setup listen event handlers.
    start_accept();

    // Queue listener for loop.
    m_acceptor->listen();
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
