#include "NetworkConnector.h"
#include "address_utils.h"

NetworkConnector::NetworkConnector(const std::shared_ptr<uvw::Loop> &loop) : m_loop(loop)
{
}

void NetworkConnector::do_connect(const std::string &address,
                                  uint16_t port)
{
    std::vector<uvw::Addr> addresses = resolve_address(address, port, m_loop);

    if(addresses.size() == 0) {
        connect_callback(nullptr);
        return;
    }

    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        m_socket->connect(*it);
    }
}

void NetworkConnector::connect(const std::string &address, unsigned int default_port, ConnectCallback callback)
{
    connect_callback = callback;

    m_socket = m_loop->resource<uvw::TcpHandle>();

    m_socket->on<uvw::ConnectEvent>([this](const uvw::ConnectEvent &, uvw::TcpHandle& ) {
        this->connect_callback(this->m_socket);
    });

    do_connect(address, default_port);
}
