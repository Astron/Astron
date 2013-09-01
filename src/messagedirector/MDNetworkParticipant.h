#pragma once
#include "MessageDirector.h"

class MDNetworkParticipant : public MDParticipantInterface
{
	public:
		MDNetworkParticipant(boost::asio::ip::tcp::socket *socket);
		virtual bool handle_datagram(Datagram *dg, DatagramIterator *dgi);
	private:
		boost::asio::ip::tcp::socket *m_socket;
};
