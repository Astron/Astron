#include "NetworkConnector.h"
#include "address_utils.h"

NetworkConnector::NetworkConnector(const std::shared_ptr<uvw::Loop> &loop) : m_loop(loop)
{
}

void NetworkConnector::do_connect(const std::shared_ptr<uvw::TcpHandle> &socket, const std::string &address,
                                  uint16_t port)
{
    std::vector<uvw::Addr> addresses = resolve_address(address, port, m_loop);

    if(addresses.size() == 0) {
        connect_callback(nullptr);
        return;
    }

    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        socket->connect(*it);
    }
}

void NetworkConnector::connect(const std::string &address, unsigned int default_port, ConnectCallback callback)
{
    connect_callback = callback;

    std::shared_ptr<uvw::TcpHandle> socket = m_loop->resource<uvw::TcpHandle>();

    socket->on<uvw::ConnectEvent>([this, socket](const uvw::ConnectEvent &, uvw::TcpHandle &) {
        this->connect_callback(socket);
    });

    do_connect(socket, address, default_port);
}
