#pragma once
#include "MessageDirector.h"
#include "util/NetworkClient.h"

class MDNetworkParticipant : public MDParticipantInterface, public NetworkClient
{
	public:
		MDNetworkParticipant(boost::asio::ip::tcp::socket *socket);
		virtual void handle_datagram(Datagram &dg, DatagramIterator &dgi);
	private:
		virtual void receive_datagram(Datagram &dg);
		virtual void receive_disconnect();
};
