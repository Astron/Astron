#include "MDNetworkParticipant.h"
#include "core/global.h"
#include <boost/bind.hpp>


MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
	: MDParticipantInterface(), NetworkClient(socket)
{
}

bool MDNetworkParticipant::handle_datagram(Datagram *dg, DatagramIterator &dgi)
{
	//TODO: make this asynch
	gLogger->debug() << "Sending to downstream md" << std::endl;
	network_send(dg);
	return true;
}

void MDNetworkParticipant::network_datagram(Datagram &dg)
{
	send(&dg);
}

void MDNetworkParticipant::network_disconnect()
{
	delete this;
}