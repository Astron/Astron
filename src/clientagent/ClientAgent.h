#pragma once
#include "util/Role.h"
#include "Client.h"

#include <boost/asio.hpp>

class ClientAgent : public Role
{
	private:
		boost::asio::ip::tcp::acceptor *m_acceptor;
		std::string m_client_type;
		std::string m_server_version;
		ChannelTracker m_ct;
		LogCategory *m_log;


	public:
		ClientAgent(RoleConfig rolconfig);
		~ClientAgent();

		void start_accept();
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);
		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);
};
