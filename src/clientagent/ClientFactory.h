#pragma once
#include "Client.h"
#include "config/ConfigVariable.h"
#include <boost/asio.hpp>
#include <unordered_map>

// A BaseClientType is a common ancestor that all client factory templates inherit from.
class BaseClientType
{
  public:
    virtual Client* instantiate(ConfigNode config, ClientAgent* client_agent,
                                boost::asio::ip::tcp::socket *socket,
                                const boost::asio::ip::tcp::endpoint &remote,
                                const boost::asio::ip::tcp::endpoint &local) = 0;
    virtual Client* instantiate(ConfigNode config, ClientAgent* client_agent,
                                boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream,
                                const boost::asio::ip::tcp::endpoint &remote,
                                const boost::asio::ip::tcp::endpoint &local) = 0;
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
                                boost::asio::ip::tcp::socket *socket,
                                const boost::asio::ip::tcp::endpoint &remote,
                                const boost::asio::ip::tcp::endpoint &local)
    {
        return new T(config, client_agent, socket, remote, local);
    }

    virtual Client* instantiate(ConfigNode config, ClientAgent* client_agent,
                                boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream,
                                const boost::asio::ip::tcp::endpoint &remote,
                                const boost::asio::ip::tcp::endpoint &local)
    {
        return new T(config, client_agent, stream, remote, local);
    }
};

// The ClientFactory is a singleton that instantiates clients from a client type.
class ClientFactory
{
  public:
    static ClientFactory& singleton();

    // instantiate_client creates a new Client object of type 'client_type'.
    Client* instantiate_client(const std::string &client_type, ConfigNode config,
                               ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket,
                               const boost::asio::ip::tcp::endpoint &remote,
                               const boost::asio::ip::tcp::endpoint &local);
    Client* instantiate_client(const std::string &client_type, ConfigNode config,
                               ClientAgent* client_agent,
                               boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream,
                               const boost::asio::ip::tcp::endpoint &remote,
                               const boost::asio::ip::tcp::endpoint &local);
    // add_client_type adds a factory for client of type 'name'
    // It is called automatically when instantiating a new ClientType.
    void add_client_type(const std::string &name, BaseClientType *factory);

    // has_client_type returns true if a client handler exists for 'name'.
    bool has_client_type(const std::string &name);
  private:
    std::unordered_map<std::string, BaseClientType*> m_factories;
};
