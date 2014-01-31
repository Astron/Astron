#pragma once
#include "Client.h"
#include "core/config.h"
#include <boost/asio.hpp>
#include <unordered_map>

// A BaseClientType is a common ancestor that all client factory templates inherit from.
class BaseClientType
{
	public:
		virtual Client* instantiate(ClientConfig config, ClientAgent* client_agent,
		                            boost::asio::ip::tcp::socket *socket) = 0;
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

		virtual Client* instantiate(ClientConfig config, ClientAgent* client_agent,
		                            boost::asio::ip::tcp::socket *socket)
		{
			return new T(config, client_agent, socket);
		}
};

// The ClientFactory is a singleton that instantiates clients from a client type.
class ClientFactory
{
	public:
		static ClientFactory& singleton();

		// instantiate_client creates a new Client object of type 'client_type'.
		Client* instantiate_client(const std::string &client_type, ClientConfig config,
		                           ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket);

		// add_client_type adds a factory for client of type 'name'
		// It is called automatically when instantiating a new ClientType.
		void add_client_type(const std::string &name, BaseClientType *factory);
	private:
		std::unordered_map<std::string, BaseClientType*> m_factories;
};