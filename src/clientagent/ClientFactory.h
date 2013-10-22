#pragma once
#include "Client.h"
#include <boost/asio.hpp>
#include <unordered_map>

class BaseClientType
{
	public:
		virtual Client* instantiate(boost::asio::ip::tcp::socket *socket, LogCategory *log, 
		                            const std::string &server_version, ChannelTracker *ct) = 0;
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

		virtual Client* instantiate(boost::asio::ip::tcp::socket *socket, LogCategory *log,
		                            const std::string &server_version, ChannelTracker *ct)
		{
			return new T(socket, log, server_version, ct);
		}
};

class ClientFactory
{
	public:
		Client* instantiate_client(const std::string &client_type, boost::asio::ip::tcp::socket *socket,
		                     LogCategory *log, const std::string &server_version, ChannelTracker *ct);
		static ClientFactory singleton;

		void add_client_type(const std::string &name, BaseClientType *factory);
	private:
		std::unordered_map<std::string, BaseClientType*> m_factories;
};