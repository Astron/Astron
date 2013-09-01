#include "MDNetworkParticipant.h"


MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
	: MDParticipantInterface(), m_socket(socket)
{
}

bool MDNetworkParticipant::handle_datagram(Datagram *dg, DatagramIterator *dgi)
{
	return true;
}