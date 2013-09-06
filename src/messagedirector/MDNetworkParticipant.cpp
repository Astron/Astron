#include "MDNetworkParticipant.h"
#include "core/global.h"
#include "core/messages.h"
#include <boost/bind.hpp>

#define MDLogger MessageDirector::singleton.logger()

MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
	: MDParticipantInterface(), NetworkClient(socket)
{
}

bool MDNetworkParticipant::handle_datagram(Datagram &dg, DatagramIterator &dgi)
{
	//TODO: make this asynch
	MDLogger.spam() << "MDNetworkParticipant sending to downstream MD" << std::endl;
	network_send(dg);
	return true;
}

void MDNetworkParticipant::network_datagram(Datagram &dg)
{
	DatagramIterator dgi(dg);
	unsigned short channels = dgi.read_uint8();
	if(channels == 1 && dgi.read_uint64() == CONTROL_MESSAGE)
	{
		unsigned short msg_type = dgi.read_uint16();
		switch(msg_type)
		{
			case CONTROL_ADD_CHANNEL:
			{
				MessageDirector::singleton.subscribe_channel(this, dgi.read_uint64());
			}
			break;
			case CONTROL_REMOVE_CHANNEL:
			{
				MessageDirector::singleton.unsubscribe_channel(this, dgi.read_uint64());
			}
			break;
			case CONTROL_ADD_RANGE:
			{
				channel_t lo = dgi.read_uint64();
				channel_t hi = dgi.read_uint64();
				MessageDirector::singleton.subscribe_range(this, lo, hi);
			}
			break;
			case CONTROL_REMOVE_RANGE:
			{
				channel_t lo = dgi.read_uint64();
				channel_t hi = dgi.read_uint64();
				MessageDirector::singleton.unsubscribe_range(this, lo, hi);
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
				MDLogger.error() << "MDNetworkParticipant got unknown control message, type : " << msg_type << std::endl;
		}
		return;
	}
	send(dg);
}

void MDNetworkParticipant::network_disconnect()
{
	delete this;
}