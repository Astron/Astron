#include "NetworkAcceptor.h"
#include "address_utils.h"
#include <boost/bind.hpp>

NetworkAcceptor::NetworkAcceptor(boost::asio::io_service& io_service) :
    m_io_service(io_service),
    m_acceptor(io_service),
    m_started(false)
{
}

boost::system::error_code NetworkAcceptor::bind(const std::string &address,
        unsigned int default_port)
{
    boost::system::error_code ec;

    auto addresses = resolve_address(address, default_port, m_io_service, ec);
    if(ec.value() != 0) {
        return ec;
    }

    for(const auto& it : addresses) {
        if(m_acceptor.is_open()) {
            m_acceptor.close();
        }

        m_acceptor.open(it.protocol(), ec);
        if(ec.value() != 0) {
            continue;
        }

        m_acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
        if(ec.value() != 0) {
            continue;
        }

        m_acceptor.bind(it, ec);
        if(ec.value() == 0) {
            break;
        }
    }
    if(ec.value() != 0) {
        return ec;
    }

    m_acceptor.listen(tcp::socket::max_connections, ec);
    if(ec.value() != 0) {
        return ec;
    }

    return ec;
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

    m_acceptor.cancel();
}
