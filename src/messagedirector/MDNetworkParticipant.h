#pragma once
#include "MessageDirector.h"
#include "util/NetworkClient.h"
#include <vector>

class MDNetworkParticipant : public MDParticipantInterface, public NetworkClient
{
public:
	MDNetworkParticipant(boost::asio::ip::tcp::socket *socket);
	virtual bool handle_datagram(Datagram &dg, DatagramIterator &dgi);
private:
	virtual void network_datagram(Datagram &dg);
	virtual void network_disconnect();
};
