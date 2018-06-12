#pragma once
#include "MessageDirector.h"
#include "net/NetworkClient.h"
#include "net/NetworkConnector.h"

// All MDUpstreams must be thread-safe. This class does not need a lock, however,
// because all of its operations are based on local variables and the functions
// of NetworkClient (which are themselves thread-safe)
class MDNetworkUpstream : public NetworkHandler, public MDUpstream
{
  public:
    MDNetworkUpstream(MessageDirector *md);

    void connect(const std::string &address);
    void on_connect(const std::shared_ptr<uvw::TcpHandle> &socket);
    void on_connect_error(const uvw::ErrorEvent& evt);

    // Queueing interfaces for datagrams pending being sent upstream.
    void send_datagram(DatagramHandle dg);
    void flush_send_queue();

    // Interfaces that MDUpstream needs us to implement:
    virtual void subscribe_channel(channel_t c);
    virtual void unsubscribe_channel(channel_t c);
    virtual void subscribe_range(channel_t lo, channel_t hi);
    virtual void unsubscribe_range(channel_t lo, channel_t hi);
    virtual void handle_datagram(DatagramHandle dg);

    // Interfaces that NetworkClient needs us to implement:
    virtual void initialize()
    {
        // Stub method for NetworkClient.
    }

    virtual void receive_datagram(DatagramHandle dg);
    virtual void receive_disconnect(const uvw::ErrorEvent &evt);

  private:
    MessageDirector *m_message_director;
    std::shared_ptr<NetworkClient> m_client;
    std::shared_ptr<NetworkConnector> m_connector;
    std::mutex m_messages_lock;
    std::queue<DatagramHandle> m_messages;
    bool m_initialized = false;
    bool m_is_sending = false;
};
