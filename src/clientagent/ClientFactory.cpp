#include "ClientFactory.h"

BaseClientType::BaseClientType(const std::string &name)
{
	ClientFactory::singleton().add_client_type(name, this);
}


ClientFactory& ClientFactory::singleton()
{
	static ClientFactory* fact = new ClientFactory();
	return *fact;
}

// add_client_type adds a factory for client of type 'name'
// It is called automatically when instantiating a new ClientType.
void ClientFactory::add_client_type(const std::string &name, BaseClientType *factory)
{
	m_factories[name] = factory;
}

// instantiate_client creates a new Client object of type 'client_type'.
Client* ClientFactory::instantiate_client(const std::string &client_type, ClientConfig config,
		ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket)
{
	if(m_factories.find(client_type) != m_factories.end())
	{
		return m_factories[client_type]->instantiate(config, client_agent, socket);
	}
	return NULL;
}