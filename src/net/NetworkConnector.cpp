#include "NetworkConnector.h"

NetworkConnector::NetworkConnector(boost::asio::io_service &io_service) : m_io_service(io_service)
{
}

tcp::socket *NetworkConnector::connect(const std::string &address,
                                       unsigned int /*default_port*/,
                                       boost::system::error_code &ec)
{
	std::string str_ip = address;
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));

	tcp::resolver resolver(m_io_service);
	tcp::resolver::query query(str_ip, str_port);
	tcp::resolver::iterator it = resolver.resolve(query);

	tcp::socket* socket = new tcp::socket(m_io_service);
	socket->connect(*it, ec);

	if(ec.value() != 0)
	{
		delete socket;
		return nullptr;
	}

	return socket;
}

ssl::stream<tcp::socket> *NetworkConnector::connect(const std::string &address,
	unsigned int /*default_port*/, ssl::context *ctx, boost::system::error_code &ec)
{
	std::string str_ip = address;
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));

	tcp::resolver resolver(m_io_service);
	tcp::resolver::query query(str_ip, str_port);
	tcp::resolver::iterator it = resolver.resolve(query);

	ssl::stream<tcp::socket> *socket = new ssl::stream<tcp::socket>(m_io_service, *ctx);
	socket->next_layer().connect(*it, ec);

	if(ec.value() != 0)
	{
		delete socket;
		return nullptr;
	}

	return socket;
}
