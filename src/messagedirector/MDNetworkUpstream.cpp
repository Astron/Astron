#include "MDNetworkUpstream.h"
#include "MessageDirector.h"
#include "net/SocketWrapper.h"
#include "core/global.h"
#include "core/msgtypes.h"

MDNetworkUpstream::MDNetworkUpstream(MessageDirector *md) :
    m_message_director(md), m_client(nullptr),
    m_connector(nullptr)
{

}

void MDNetworkUpstream::connect(const std::string &address)
{
    m_connector.reset(new NetworkConnector([this](
                        SocketPtr socket,
                        const uvw::ErrorEvent &error) {
        this->connect_callback(socket, error);
    }));
    m_connector->connect(address, 7199);
}

void MDNetworkUpstream::connect_callback(SocketPtr socket, const uvw::ErrorEvent &error)
{
    if (!socket)
    {
        // Failed to connect
        receive_disconnect(error);
        return;
    }

    // Initialize m_client
    m_connector = nullptr;
    m_client = std::make_shared<NetworkClient>(this);
    m_client->initialize(socket);

    // Start listening for downstream connections
    m_message_director->start_listening();
}

void MDNetworkUpstream::subscribe_channel(channel_t c)
{
    DatagramPtr dg = Datagram::create(CONTROL_ADD_CHANNEL);
    dg->add_channel(c);
    m_client->send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_channel(channel_t c)
{
    DatagramPtr dg = Datagram::create(CONTROL_REMOVE_CHANNEL);
    dg->add_channel(c);
    m_client->send_datagram(dg);
}

void MDNetworkUpstream::subscribe_range(channel_t lo, channel_t hi)
{
    DatagramPtr dg = Datagram::create(CONTROL_ADD_RANGE);
    dg->add_channel(lo);
    dg->add_channel(hi);
    m_client->send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_range(channel_t lo, channel_t hi)
{
    DatagramPtr dg = Datagram::create(CONTROL_REMOVE_RANGE);
    dg->add_channel(lo);
    dg->add_channel(hi);
    m_client->send_datagram(dg);
}

void MDNetworkUpstream::handle_datagram(DatagramHandle dg)
{
    m_client->send_datagram(dg);
}

void MDNetworkUpstream::receive_datagram(DatagramHandle dg)
{
    m_message_director->receive_datagram(dg);
}

void MDNetworkUpstream::receive_disconnect(const uvw::ErrorEvent &error)
{
    m_message_director->receive_disconnect(error);
}
