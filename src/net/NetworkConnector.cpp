#include "NetworkConnector.h"
#include "address_utils.h"

NetworkConnector::NetworkConnector(boost::asio::io_service &io_service) : m_io_service(io_service)
{
}

void NetworkConnector::do_connect(tcp::socket &socket, const std::string &address,
                                  uint16_t port, boost::system::error_code &ec)
{
    std::vector<tcp::endpoint> addresses = resolve_address(address, port, m_io_service, ec);

    if(ec.value() != 0) {
        return;
    }

    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        socket.connect(*it, ec);

        if(ec.value() == 0) {
            return;
        }
    }
}

tcp::socket *NetworkConnector::connect(const std::string &address,
                                       unsigned int default_port,
                                       boost::system::error_code &ec)
{
    tcp::socket* socket = new tcp::socket(m_io_service);
    do_connect(*socket, address, default_port, ec);

    if(ec.value() != 0) {
        delete socket;
        return nullptr;
    }

    return socket;
}

ssl::stream<tcp::socket> *NetworkConnector::connect(const std::string &address,
        unsigned int default_port, ssl::context *ctx, boost::system::error_code &ec)
{
    ssl::stream<tcp::socket> *socket = new ssl::stream<tcp::socket>(m_io_service, *ctx);
    do_connect(socket->next_layer(), address, default_port, ec);

    if(ec.value() != 0) {
        delete socket;
        return nullptr;
    }

    return socket;
}
