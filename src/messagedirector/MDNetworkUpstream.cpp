#include "MDNetworkUpstream.h"
#include "MessageDirector.h"
#include "net/NetworkConnector.h"
#include "core/global.h"
#include "core/msgtypes.h"

using boost::asio::ip::tcp;

MDNetworkUpstream::MDNetworkUpstream(MessageDirector *md) :
    m_message_director(md)
{

}

boost::system::error_code MDNetworkUpstream::connect(const std::string &address)
{
    NetworkConnector connector(io_service);
    boost::system::error_code ec;
    tcp::socket *socket = connector.connect(address, 7199, ec);

    if(socket) {
        set_socket(socket);
    }

    return ec;
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

void MDNetworkUpstream::receive_disconnect()
{
    m_message_director->receive_disconnect();
}
