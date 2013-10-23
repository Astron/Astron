#include "ClientFactory.h"

ClientFactory ClientFactory::singleton;

BaseClientType::BaseClientType(const std::string &name)
{
	ClientFactory::singleton.add_client_type(name, this);
}

void ClientFactory::add_client_type(const std::string &name, BaseClientType *factory)
{
	m_factories[name] = factory;
}

Client* ClientFactory::instantiate_client(const std::string &client_type, ClientAgent* client_agent,
                                          boost::asio::ip::tcp::socket *socket)
{
	if(m_factories.find(client_type) != m_factories.end())
	{
		return m_factories[client_type]->instantiate(client_agent, socket);
	}
	return NULL;
}