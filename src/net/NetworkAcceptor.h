#pragma once
#include <boost/asio.hpp>
#include <functional>

using boost::asio::ip::tcp;

typedef std::function<void(tcp::socket*)> AcceptorCallback;

class NetworkAcceptor
{
	public:
		NetworkAcceptor(boost::asio::io_service &io_service,
		                AcceptorCallback &callback);

		// Parses the string "address" and binds to it. If no port is specified
		// as part of the address, it will use default_port.
		boost::system::error_code bind(const std::string &address,
		                               unsigned int default_port);

		void start();
		void stop();

	private:
		void start_accept();
		void handle_accept(tcp::socket *socket, const boost::system::error_code &ec);

		boost::asio::io_service &m_io_service;
		AcceptorCallback m_callback;

		tcp::acceptor m_acceptor;

		bool m_started;
};
