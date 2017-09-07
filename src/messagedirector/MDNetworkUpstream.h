#pragma once
#include "MessageDirector.h"
#include "net/NetworkClient.h"
#include <boost/asio.hpp>

// All MDUpstreams must be thread-safe. This class does not need a lock, however,
// because all of its operations are based on local variables and the functions
// of NetworkClient (which are themselves thread-safe)
class MDNetworkUpstream : public NetworkHandler, public MDUpstream
{
  public:
    MDNetworkUpstream(MessageDirector *md);

    void connect(const std::string &address);
    void on_connect(const std::shared_ptr<uvw::TcpHandle> &socket);

    // Interfaces that MDUpstream needs us to implement:
    virtual void subscribe_channel(channel_t c);
    virtual void unsubscribe_channel(channel_t c);
    virtual void subscribe_range(channel_t lo, channel_t hi);
    virtual void unsubscribe_range(channel_t lo, channel_t hi);
    virtual void handle_datagram(DatagramHandle dg);

    // Interfaces that NetworkClient needs us to implement:
    virtual void receive_datagram(DatagramHandle dg);
    virtual void receive_disconnect(const uvw::ErrorEvent &evt);

  private:
    MessageDirector *m_message_director;
    std::shared_ptr<NetworkClient> m_client;
};
