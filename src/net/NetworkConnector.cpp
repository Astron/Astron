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
        this->m_socket->connect(*it);
    }
}

void NetworkConnector::connect(const std::string &address, unsigned int default_port, ConnectCallback callback)
{
    connect_callback = callback;

    this->m_socket = m_loop->resource<uvw::TcpHandle>();

    this->m_socket->on<uvw::ConnectEvent>([connector = shared_from_this()](const uvw::ConnectEvent &, uvw::TcpHandle& ) {
        connector->connect_callback(connector->m_socket);
    });

    do_connect(address, default_port);
}
