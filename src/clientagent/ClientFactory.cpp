#include "ClientFactory.h"

BaseClientType::BaseClientType(const std::string &name)
{
    ClientFactory::singleton().add_client_type(name, this);
}


ClientFactory& ClientFactory::singleton()
{
    static ClientFactory fact;
    return fact;
}

// add_client_type adds a factory for client of type 'name'
// It is called automatically when instantiating a new ClientType.
void ClientFactory::add_client_type(const std::string &name, BaseClientType *factory)
{
    m_factories[name] = factory;
}

// has_client_type returns true if a client handler exists for 'name'.
bool ClientFactory::has_client_type(const std::string &name)
{
    return m_factories.find(name) != m_factories.end();
}

// instantiate_client creates a new Client object of type 'client_type'.
Client* ClientFactory::instantiate_client(const std::string &client_type, ConfigNode config,
        ClientAgent* client_agent, const std::shared_ptr<uvw::TcpHandle> &socket,
        const uvw::Addr &remote, const uvw::Addr &local, const bool haproxy_mode)
{
    if(m_factories.find(client_type) != m_factories.end()) {
        return m_factories[client_type]->instantiate(config, client_agent, socket, remote, local, haproxy_mode);
    }
    return nullptr;
}
