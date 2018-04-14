#include "core/global.h"
#include "NetworkAcceptor.h"
#include "address_utils.h"


NetworkAcceptor::NetworkAcceptor(AcceptorErrorCallback err_callback) :
    m_loop(g_loop),
    m_acceptor(nullptr),
    m_started(false),
    m_haproxy_mode(false),
    m_err_callback(err_callback)
{
}

void NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    m_acceptor = m_loop->resource<uvw::TcpHandle>();
    m_acceptor->simultaneousAccepts(true);

    std::vector<uvw::Addr> addresses = resolve_address(address, default_port, m_loop);

    if(addresses.size() == 0) {
        this->m_err_callback(uvw::ErrorEvent{(int)UV_EADDRNOTAVAIL});
        return;
    }

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
