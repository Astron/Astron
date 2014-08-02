#include "NetworkAcceptor.h"
#include <boost/bind.hpp>

NetworkAcceptor::NetworkAcceptor(boost::asio::io_service& io_service) :
    m_io_service(io_service),
    m_acceptor(io_service),
    m_started(false)
{
}

boost::system::error_code NetworkAcceptor::bind(const std::string &address,
        unsigned int /*default_port*/)
{
    std::string str_ip = address;
    std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
    // TODO: Use default port of str_port is empty.
    str_ip = str_ip.substr(0, str_ip.find(':', 0));

    tcp::resolver resolver(m_io_service);
    tcp::resolver::query query(str_ip, str_port);

    boost::system::error_code ec;
    tcp::resolver::iterator it = resolver.resolve(query, ec);
    if(ec.value() != 0) {
        return ec;
    }

    tcp::endpoint ep = *it;

    m_acceptor.open(ep.protocol(), ec);
    if(ec.value() != 0) {
        return ec;
    }

    m_acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
    if(ec.value() != 0) {
        return ec;
    }

    m_acceptor.bind(ep, ec);
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
