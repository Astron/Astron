#include "NetworkAcceptor.h"
#include "address_utils.h"
#include "core/shutdown.h"

#include <iostream>

NetworkAcceptor::NetworkAcceptor() : m_started(false)
{
    auto loop = uvw::Loop::getDefault();
    m_acceptor = loop->resource<uvw::TcpHandle>();
    m_acceptor->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& error, const auto&) {
        this->error_callback(error);
    });
}

bool NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    auto addr = resolve_address(address, default_port);
    if(addr.ip == "") {
        return false;
    }

    m_acceptor->bind(addr);
    return true;
}

void NetworkAcceptor::start()
{
    if(m_started) {
        // Already started, start() was called twice!
        return;
    }

    m_started = true;

    start_accept();
}

void NetworkAcceptor::stop()
{
    if(!m_started) {
        // Already stopped, stop() was called twice!
        return;
    }

    m_started = false;

    m_acceptor->stop();
}

void NetworkAcceptor::error_callback(const uvw::ErrorEvent& err)
{
    std::cerr << "Acceptor error: " << err.what() << std::endl;
    astron_shutdown(1);
}
