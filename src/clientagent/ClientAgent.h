#pragma once
#include "core/Role.h"
#include "core/global.h"
#include "Client.h"

#include <unordered_map>
#include <memory>

extern RoleConfigGroup clientagent_config;
extern KeyedConfigGroup ca_client_config;
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

class ClientAgent final : public Role
{
    friend class Client;

  public:
    ClientAgent(RoleConfig rolconfig);

    void init_metrics();

    // handle_tcp generates a new Client object from a raw tcp connection.
    void handle_tcp(const std::shared_ptr<uvw::TcpHandle> &socket,
                    const uvw::Addr &remote,
                    const uvw::Addr &local,
                    const bool haproxy_mode);

    void handle_error(const uvw::ErrorEvent& evt);

    // handle_datagram handles Datagrams received from the message director.
    // Currently the ClientAgent does not handle any datagrams,
    // and delegates everything to the Client objects.
    void handle_datagram(DatagramHandle in_dg, DatagramIterator &dgi);

    const std::string& get_version() const
    {
        return m_server_version;
    }

    uint32_t get_hash() const
    {
        return m_hash;
    }

    LogCategory *log()
    {
        return m_log.get();
    }

    void add_client(channel_t channel, Client* client)
    {
        m_clients[channel] = client;
        update_client_gauge();
    }

    void remove_client(channel_t channel)
    {
        m_clients.erase(channel);
        update_client_gauge();
    }

    void update_client_gauge()
    {
        if(m_client_count_gauge)
            m_client_count_gauge->Set(m_clients.size());
    }

    void report_interest_timeout()
    {
        if(m_interest_timeout_ctr)
            m_interest_timeout_ctr->Increment();
    }

    void report_interest_time(uvw::TimerHandle::Time time)
    {
        if(m_interest_time_hist)
            m_interest_time_hist->Observe(time.count());
    }
private:
    std::unique_ptr<NetworkAcceptor> m_net_acceptor;
    std::string m_client_type;
    std::string m_server_version;
    ChannelTracker m_ct;
    ConfigNode m_clientconfig;
    std::unique_ptr<LogCategory> m_log;
    uint32_t m_hash;
    std::unordered_map<channel_t, Client*> m_clients;
    unsigned long m_interest_timeout;

    // Our Prometheus metrics:
    prometheus::Family<prometheus::Gauge>* m_client_count_builder = nullptr;
    prometheus::Gauge* m_client_count_gauge = nullptr;
    prometheus::Family<prometheus::Histogram>* m_interest_time_builder = nullptr;
    prometheus::Histogram* m_interest_time_hist = nullptr;
    prometheus::Family<prometheus::Counter>* m_interest_timeout_builder = nullptr;
    prometheus::Counter* m_interest_timeout_ctr = nullptr;
};
