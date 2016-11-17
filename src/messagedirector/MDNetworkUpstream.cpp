#include "MDNetworkUpstream.h"
#include "MessageDirector.h"
#include "net/NetworkConnector.h"
#include "core/global.h"
#include "core/msgtypes.h"

using boost::asio::ip::tcp;

MDNetworkUpstream::MDNetworkUpstream(MessageDirector *md) :
    m_message_director(md), m_client(std::make_shared<NetworkClient>(this))
{

}

boost::system::error_code MDNetworkUpstream::connect(const std::string &address)
{
    NetworkConnector connector(io_service);
    boost::system::error_code ec;
    tcp::socket *socket = connector.connect(address, 7199, ec);

    if(socket) {
        m_client->initialize(socket);
    }

    return ec;
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

void MDNetworkUpstream::receive_disconnect(const boost::system::error_code &ec)
{
    m_message_director->receive_disconnect(ec);
}
