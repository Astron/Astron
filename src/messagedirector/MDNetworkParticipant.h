#pragma once
#include "MessageDirector.h"
#include "net/NetworkClient.h"

class MDNetworkParticipant : public MDParticipantInterface, public NetworkHandler
{
  public:
    MDNetworkParticipant(const std::shared_ptr<uvw::TcpHandle> &socket);
    ~MDNetworkParticipant();
    virtual void initialize()
    {
        // Stub method for NetworkClient.
    }

    virtual void handle_datagram(DatagramHandle dg, DatagramIterator &dgi);
  private:
    virtual void receive_datagram(DatagramHandle dg);
    virtual void receive_disconnect(const uvw::ErrorEvent &evt);

    std::shared_ptr<NetworkClient> m_client;
};
