#pragma once
#include "Client.h"
#include <boost/asio.hpp>
#include <unordered_map>

class BaseClientType
{
	public:
		virtual Client* instantiate(ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket) = 0;
	protected:
		BaseClientType(const std::string &name);
};

template <class T>
class ClientType : public BaseClientType
{
	public:
		ClientType(const std::string &name) : BaseClientType(name)
		{
		}

		virtual Client* instantiate(ClientAgent* client_agent, boost::asio::ip::tcp::socket *socket)
		{
			return new T(client_agent, socket);
		}
};

class ClientFactory
{
	public:
		Client* instantiate_client(const std::string &client_type, ClientAgent* client_agent,
		                           boost::asio::ip::tcp::socket *socket);
		static ClientFactory singleton;

		void add_client_type(const std::string &name, BaseClientType *factory);
	private:
		std::unordered_map<std::string, BaseClientType*> m_factories;
};