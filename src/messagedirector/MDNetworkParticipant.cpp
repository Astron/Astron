#include "MDNetworkParticipant.h"
#include "core/global.h"
#include "core/msgtypes.h"

MDNetworkParticipant::MDNetworkParticipant(const std::shared_ptr<uvw::TcpHandle> &socket)
    : MDParticipantInterface(), m_client(std::make_shared<NetworkClient>(this))
{
    set_con_name("Network Participant");

    m_client->initialize(socket);
}

MDNetworkParticipant::~MDNetworkParticipant()
{
    m_client->disconnect();
}

void MDNetworkParticipant::handle_datagram(DatagramHandle dg, DatagramIterator&)
{
    logger().trace() << "MDNetworkParticipant sending to downstream MD" << std::endl;
    m_client->send_datagram(dg);
}

void MDNetworkParticipant::receive_datagram(DatagramHandle dg)
{
    DatagramIterator dgi(dg);
    try {
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
                    channel_t sender = dgi.read_channel();
                    add_post_remove(sender, dgi.read_datagram());
                    break;
                }
                case CONTROL_CLEAR_POST_REMOVES: {
                    clear_post_removes(dgi.read_channel());
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
                case CONTROL_LOG_MESSAGE: {
                    log_message(dgi.read_blob());
                    break;
                }
                default:
                    logger().error() << "MDNetworkParticipant got unknown control message"
                                     << "with message type: " << msg_type << std::endl;
            }
            return;
        }
    } catch(const DatagramIteratorEOF &) {
        logger().error() << "MDNetworkParticipant received a truncated datagram." << std::endl;
        terminate();
        return;
    }

    route_datagram(dg);
}

void MDNetworkParticipant::receive_disconnect(const uvw::ErrorEvent &evt)
{
    logger().info() << "Lost connection from "
                    << m_client->get_remote().ip << ":"
                    << m_client->get_remote().port << ": "
                    << evt.what() << std::endl;
    terminate();
}
