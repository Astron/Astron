#include "MDNetworkParticipant.h"
#include "core/global.h"
#include "core/msgtypes.h"
#include <boost/bind.hpp>

MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
    : MDParticipantInterface(), NetworkClient(socket)
{
    set_con_name("Network Participant");
}

void MDNetworkParticipant::handle_datagram(DatagramHandle dg, DatagramIterator&)
{
    logger().trace() << "MDNetworkParticipant sending to downstream MD" << std::endl;
    try {
        send_datagram(dg);
    } catch(const boost::system::system_error &e) {
        logger().warning() << "Received a system error while sending a datagram to a network "
                           "participant (the participant may have lost connection)." << std::endl;
        return;
    }
}

void MDNetworkParticipant::receive_datagram(DatagramHandle dg)
{
    DatagramIterator dgi(dg);
    uint16_t channels = dgi.read_uint8();
    if(channels == 1 && dgi.read_channel() == CONTROL_MESSAGE) {
        uint16_t msg_type = dgi.read_uint16();
        switch(msg_type) {
        case CONTROL_ADD_CHANNEL: {
            subscribe_channel(dgi.read_channel());
            break;
        }
        case CONTROL_REMOVE_CHANNEL: {
            unsubscribe_channel(dgi.read_channel());
            break;
        }
        case CONTROL_ADD_RANGE: {
            channel_t lo = dgi.read_channel();
            channel_t hi = dgi.read_channel();
            subscribe_range(lo, hi);
            break;
        }
        case CONTROL_REMOVE_RANGE: {
            channel_t lo = dgi.read_channel();
            channel_t hi = dgi.read_channel();
            unsubscribe_range(lo, hi);
            break;
        }
        case CONTROL_ADD_POST_REMOVE: {
            add_post_remove(dgi.read_datagram());
            break;
        }
        case CONTROL_CLEAR_POST_REMOVE: {
            clear_post_removes();
            break;
        }
        case CONTROL_SET_CON_NAME: {
            set_con_name(dgi.read_string());
            break;
        }
        case CONTROL_SET_CON_URL: {
            set_con_url(dgi.read_string());
            break;
        }
        default:
            logger().error() << "MDNetworkParticipant got unknown control message, type : "
                             << msg_type << std::endl;
        }
        return;
    }
    route_datagram(dg);
}

void MDNetworkParticipant::receive_disconnect()
{
    delete this;
}
