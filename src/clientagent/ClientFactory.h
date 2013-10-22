#pragma once
#include <boost/asio.hpp>

#include "core/global.h"

class Client;
class ChannelTracker;
class LogCategory;

class BaseClientType
{
	public:
		BaseClientType(unsigned char priority);
		virtual Client* instantiate(boost::asio::ip::tcp::socket *socket, LogCategory *log, 
                                    std::string server_version, ChannelTracker *ct) = 0;
		unsigned char priority();
	private:
		unsigned char m_priority;
};

template <class T>
class ClientType : public BaseClientType
{
	public:
		ClientType(unsigned char priority) : BaseClientType(priority)
		{
		}

		virtual Client* instantiate(boost::asio::ip::tcp::socket *socket, LogCategory *log,
                                    std::string server_version, ChannelTracker *ct)
		{
			return new T(socket, log, server_version, ct);
		}
};

class ClientFactory
{
	public:
		static ClientFactory& get_singleton();
		void set_client_type(BaseClientType *client_type);
		Client* create(boost::asio::ip::tcp::socket *socket, LogCategory *log,
                       std::string server_version, ChannelTracker *ct);
	private:
		ClientFactory();

		BaseClientType* m_client_type;
};