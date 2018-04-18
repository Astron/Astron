#pragma once
#include "Client.h"
#include "config/ConfigVariable.h"
#include <unordered_map>

// A BaseClientType is a common ancestor that all client factory templates inherit from.
class BaseClientType
{
  public:
    virtual Client* instantiate(ConfigNode config, ClientAgent* client_agent,
                                const std::shared_ptr<uvw::TcpHandle> &socket,
                                const uvw::Addr &remote,
                                const uvw::Addr &local,
                                const bool haproxy_mode) = 0;
  protected:
    BaseClientType(const std::string &name);
};

// A ClientType is the factory for a particular client.
// Each new client should declare a ClientType<ClientClass>("ClientName");
template <class T>
class ClientType : public BaseClientType
{
  public:
    ClientType(const std::string &name) : BaseClientType(name)
    {
    }

    virtual Client* instantiate(ConfigNode config, ClientAgent* client_agent,
                                const std::shared_ptr<uvw::TcpHandle> &socket,
                                const uvw::Addr &remote,
                                const uvw::Addr &local,
                                const bool haproxy_mode)
    {
        return new T(config, client_agent, socket, remote, local, haproxy_mode);
    }
};

// The ClientFactory is a singleton that instantiates clients from a client type.
class ClientFactory
{
  public:
    static ClientFactory& singleton();

    // instantiate_client creates a new Client object of type 'client_type'.
    Client* instantiate_client(const std::string &client_type, ConfigNode config,
                               ClientAgent* client_agent, const std::shared_ptr<uvw::TcpHandle> &socket,
                               const uvw::Addr &remote,
                               const uvw::Addr &local,
                               const bool haproxy_mode);
    // add_client_type adds a factory for client of type 'name'
    // It is called automatically when instantiating a new ClientType.
    void add_client_type(const std::string &name, BaseClientType *factory);

    // has_client_type returns true if a client handler exists for 'name'.
    bool has_client_type(const std::string &name);
  private:
    std::unordered_map<std::string, BaseClientType*> m_factories;
};
