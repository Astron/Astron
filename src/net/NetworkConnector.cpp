#include "NetworkConnector.h"
#include "address_utils.h"

NetworkConnector::NetworkConnector(ConnectCallback callback) : m_callback(callback),
  m_socket(nullptr)
{
}

NetworkConnector::~NetworkConnector()
{
    m_socket = nullptr;
}

void NetworkConnector::do_connect(const std::string &address, uint16_t port)
{
    m_socket->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent &error, uvw::TcpHandle &) {
        this->m_callback(nullptr, error);
    });
    m_socket->once<uvw::ConnectEvent>([this](const uvw::ConnectEvent &, uvw::TcpHandle &) {
        uvw::ErrorEvent noerror(0);
        auto sock = std::make_shared<SocketWrapper>(this->m_socket);
        sock->initialize();
        this->m_callback(sock, noerror);
    });

    auto addr = resolve_address(address, port);
    if (addr.ip == "") {
        m_callback(nullptr, uvw::ErrorEvent{(int)UV_EINVAL});
        return;
    }

    m_socket->connect(addr);
}

void NetworkConnector::connect(const std::string &address, unsigned int default_port)
{
    auto loop = uvw::Loop::getDefault();
    m_socket = loop->resource<uvw::TcpHandle>();
    do_connect(address, default_port);
}
