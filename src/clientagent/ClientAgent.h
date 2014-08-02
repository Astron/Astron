#pragma once
#include "core/Role.h"
#include "Client.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

extern RoleConfigGroup clientagent_config;
extern ConfigGroup ca_client_config;
extern ConfigVariable<std::string> ca_client_type;

// A ChannelTracker is used to keep track of available and allocated channels that
// the ClientAgent can use to assign to new Clients.
// TODO: Consider moving to util/ this class might be reusable in other roles that utilize ranges.
class ChannelTracker
{
  public:
    ChannelTracker(channel_t min = INVALID_CHANNEL, channel_t max = INVALID_CHANNEL);

    channel_t alloc_channel();
    void free_channel(channel_t channel);

  private:
    channel_t m_next;
    channel_t m_max;
    std::queue<channel_t> m_unused_channels;
};

class ClientAgent : public Role
{
    friend class Client;

  public:
    ClientAgent(RoleConfig rolconfig);
    ~ClientAgent();

    // handle_tcp generates a new Client object from a raw tcp connection.
    void handle_tcp(boost::asio::ip::tcp::socket *socket);

    // handle_ssl generates a new Client object from an ssl stream.
    void handle_ssl(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream);

    // handle_datagram handles Datagrams received from the message director.
    // Currently the ClientAgent does not handle any datagrams,
    // and delegates everything to the Client objects.
    void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

    // ssl_password_callback prompts for password on stdin if the cert/key has a password
    std::string ssl_password_callback();

    const std::string& get_version() const
    {
        return m_server_version;
    }

    uint32_t get_hash() const
    {
        return m_hash;
    }

  private:
    NetworkAcceptor *m_net_acceptor;
    std::string m_client_type;
    std::string m_server_version;
    ChannelTracker m_ct;
    ConfigNode m_clientconfig;
    LogCategory *m_log;
    uint32_t m_hash;

    boost::asio::ssl::context m_ssl_ctx;
    std::string m_ssl_cert;
    std::string m_ssl_key;
};
