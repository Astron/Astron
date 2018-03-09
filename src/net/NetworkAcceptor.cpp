#include "NetworkAcceptor.h"
#include "address_utils.h"

NetworkAcceptor::NetworkAcceptor() :
    m_loop(nullptr),
    m_acceptor(nullptr),
    m_started(false)
{
}

void NetworkAcceptor::listener_thread()
{
    printf("loop is running\n");
    m_loop->run();
    printf("loop is dead\n");
    stop();
}

void NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    m_loop = uvw::Loop::create();
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

    // Sets up the handlers for accepting peers.
    start_accept();

    // Start listening for inbound connections.
    m_acceptor->listen();

    // Start loop's thread.
    m_thread.reset(new std::thread(std::bind(&NetworkAcceptor::listener_thread, this)));
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
