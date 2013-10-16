#include "MDNetworkParticipant.h"
#include "core/global.h"
#include "core/messages.h"
#include <boost/bind.hpp>

MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
	: MDParticipantInterface(), NetworkClient(socket)
{
}

void MDNetworkParticipant::handle_datagram(Datagram &dg, DatagramIterator &dgi)
{
	logger().spam() << "MDNetworkParticipant sending to downstream MD" << std::endl;
	try
	{
		network_send(dg);
	}
	catch(boost::system::system_error &e)
	{
		logger().warning() << "Received a system error while sending a datagram to a network "
		                      "participant (the participant may have lost connection)." << std::endl;
		return;
	}
}

void MDNetworkParticipant::network_datagram(Datagram &dg)
{
	DatagramIterator dgi(dg);
	uint16_t channels = dgi.read_uint8();
	if(channels == 1 && dgi.read_uint64() == CONTROL_MESSAGE)
	{
		uint16_t msg_type = dgi.read_uint16();
		switch(msg_type)
		{
			case CONTROL_ADD_CHANNEL:
			{
				subscribe_channel(dgi.read_uint64());
			}
			break;
			case CONTROL_REMOVE_CHANNEL:
			{
				unsubscribe_channel(dgi.read_uint64());
			}
			break;
			case CONTROL_ADD_RANGE:
			{
				channel_t lo = dgi.read_uint64();
				channel_t hi = dgi.read_uint64();
				subscribe_range(lo, hi);
			}
			break;
			case CONTROL_REMOVE_RANGE:
			{
				channel_t lo = dgi.read_uint64();
				channel_t hi = dgi.read_uint64();
				unsubscribe_range(lo, hi);
			}
			break;
			case CONTROL_ADD_POST_REMOVE:
			{
				add_post_remove(dgi.read_string());
			}
			break;
			case CONTROL_CLEAR_POST_REMOVE:
			{
				clear_post_removes();
			}
			break;
			default:
				logger().error() << "MDNetworkParticipant got unknown control message, type : "
				                 << msg_type << std::endl;
		}
		return;
	}
	send(dg);
}

void MDNetworkParticipant::network_disconnect()
{
	delete this;
}
