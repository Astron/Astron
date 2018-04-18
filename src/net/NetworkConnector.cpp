#include "NetworkConnector.h"
#include "address_utils.h"
#include "core/global.h"

NetworkConnector::NetworkConnector(const std::shared_ptr<uvw::Loop> &loop) : m_loop(loop)
{
}

void NetworkConnector::do_connect(const std::string &address,
                                  uint16_t port)
{
    std::vector<uvw::Addr> addresses = resolve_address(address, port, m_loop);

    if(addresses.size() == 0) {
        if(m_connect_callback != nullptr)
            m_connect_callback(nullptr);

        return;
    }

    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        m_socket->connect(*it);
    }
}

void NetworkConnector::destroy()
{
    m_connect_callback = nullptr;
    m_err_callback = nullptr;
    m_socket = nullptr;
}

void NetworkConnector::connect(const std::string &address, unsigned int default_port,
                               ConnectCallback callback, ConnectErrorCallback err_callback)
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_connect_callback = callback;
    m_err_callback = err_callback;

    m_socket = m_loop->resource<uvw::TcpHandle>();

    m_socket->once<uvw::ConnectEvent>([self = shared_from_this()](const uvw::ConnectEvent &, uvw::TcpHandle&) {
        if(self->m_connect_callback != nullptr)
            self->m_connect_callback(self->m_socket);
    });

    m_socket->once<uvw::ErrorEvent>([self = shared_from_this()](const uvw::ErrorEvent &evt, uvw::TcpHandle&) {
        if(self->m_err_callback != nullptr)
            self->m_err_callback(evt);
    });

    do_connect(address, default_port);
}
