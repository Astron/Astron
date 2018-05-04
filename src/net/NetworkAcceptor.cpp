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
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_acceptor = m_loop->resource<uvw::TcpHandle>();
    m_acceptor->simultaneousAccepts(true);

    std::vector<uvw::Addr> addresses = resolve_address(address, default_port, m_loop);

    if(addresses.size() == 0) {
        this->m_err_callback(uvw::ErrorEvent{(int)UV_EADDRNOTAVAIL});
        return;
    }

    // Setup listen/error event handlers.
    start_accept();

    for (uvw::Addr& addr : addresses) {
        m_acceptor->bind(addr);
    }
}

void NetworkAcceptor::start()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_started) {
        // Already started, start() was called twice!
        return;
    }

    m_started = true;
    
    // Queue listener for loop.
    m_acceptor->listen();
}

void NetworkAcceptor::stop()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(!m_started) {
        // Already stopped, stop() was called twice!
        return;
    }

    m_started = false;

    m_acceptor->close();
}
