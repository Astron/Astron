#pragma once
#include "MessageDirector.h"
#include "net/NetworkClient.h"

class MDNetworkParticipant : public MDParticipantInterface, public NetworkHandler
{
  public:
    MDNetworkParticipant(SocketPtr socket);
    ~MDNetworkParticipant();
    virtual void handle_datagram(DatagramHandle dg, DatagramIterator &dgi);
  private:
    virtual void receive_datagram(DatagramHandle dg);
    virtual void receive_disconnect(const uvw::ErrorEvent &error);

    std::shared_ptr<NetworkClient> m_client;
};
