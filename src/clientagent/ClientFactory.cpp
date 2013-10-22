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

Client* ClientFactory::instantiate_client(const std::string &client_type, 
                                          boost::asio::ip::tcp::socket *socket, LogCategory *log,
                                          const std::string &server_version, ChannelTracker *ct)
{
	if(m_factories.find(client_type) != m_factories.end())
	{
		return m_factories[client_type]->instantiate(socket, log, server_version, ct);
	}
	return NULL;
}