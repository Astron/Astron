#include "MDNetworkUpstream.h"
#include "MessageDirector.h"
#include "net/NetworkConnector.h"
#include "core/global.h"
#include "core/msgtypes.h"

MDNetworkUpstream::MDNetworkUpstream(MessageDirector *md) :
    m_message_director(md), m_client(std::make_shared<NetworkClient>(this)),
    m_connector(std::make_shared<NetworkConnector>(g_loop))
{

}

void MDNetworkUpstream::connect(const std::string &address)
{
    ConnectCallback callback = std::bind(&MDNetworkUpstream::on_connect, this, std::placeholders::_1);
    ConnectErrorCallback err_callback = std::bind(&MDNetworkUpstream::on_connect_error, this, std::placeholders::_1);

    m_connector->connect(address, 7199, callback, err_callback);
}

void MDNetworkUpstream::on_connect(const std::shared_ptr<uvw::TcpHandle> &socket)
{
    if(socket == nullptr) {
        m_message_director->receive_disconnect(uvw::ErrorEvent{(int)UV_EADDRNOTAVAIL});
        exit(1);
    }

    m_client->initialize(socket);
    m_initialized = true;

    // Flush any datagrams that we tried to send out before our connection to upstream was initialised.
    assert(!m_is_sending);
    flush_send_queue();

    m_connector->destroy();
    m_connector = nullptr;
}

void MDNetworkUpstream::send_datagram(DatagramHandle dg)
{
    if(!m_initialized) {
        std::lock_guard<std::mutex> lock(m_messages_lock);
        m_messages.push(dg);
    }
    else {
        m_client->send_datagram(dg);
    }
}

void MDNetworkUpstream::flush_send_queue()
{
    if(m_is_sending) {
        return;
    }

    m_is_sending = true;

    {
        std::unique_lock<std::mutex> lock(m_messages_lock);
        while(!m_messages.empty()) {
            DatagramHandle dg = m_messages.front();
            m_messages.pop();
            m_client->send_datagram(dg);
        }
    }

    m_is_sending = false;
}

void MDNetworkUpstream::on_connect_error(const uvw::ErrorEvent& evt)
{
    m_message_director->receive_disconnect(evt);
}

void MDNetworkUpstream::subscribe_channel(channel_t c)
{
    DatagramPtr dg = Datagram::create(CONTROL_ADD_CHANNEL);
    dg->add_channel(c);
    send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_channel(channel_t c)
{
    DatagramPtr dg = Datagram::create(CONTROL_REMOVE_CHANNEL);
    dg->add_channel(c);
    send_datagram(dg);
}

void MDNetworkUpstream::subscribe_range(channel_t lo, channel_t hi)
{
    DatagramPtr dg = Datagram::create(CONTROL_ADD_RANGE);
    dg->add_channel(lo);
    dg->add_channel(hi);
    send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_range(channel_t lo, channel_t hi)
{
    DatagramPtr dg = Datagram::create(CONTROL_REMOVE_RANGE);
    dg->add_channel(lo);
    dg->add_channel(hi);
    send_datagram(dg);
}

void MDNetworkUpstream::handle_datagram(DatagramHandle dg)
{
    send_datagram(dg);
}

void MDNetworkUpstream::receive_datagram(DatagramHandle dg)
{
    m_message_director->receive_datagram(dg);
}

void MDNetworkUpstream::receive_disconnect(const uvw::ErrorEvent &evt)
{
    m_message_director->receive_disconnect(evt);
}
