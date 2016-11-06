#pragma once
#include "MessageDirector.h"
#include "net/NetworkClient.h"

class MDNetworkParticipant : public MDParticipantInterface, public NetworkHandler
{
  public:
    MDNetworkParticipant(boost::asio::ip::tcp::socket *socket);
    ~MDNetworkParticipant();
    virtual void handle_datagram(DatagramHandle dg, DatagramIterator &dgi);
  private:
    virtual void receive_datagram(DatagramHandle dg);
    virtual void receive_disconnect(const boost::system::error_code &ec);

    std::shared_ptr<NetworkClient> m_client;
};
