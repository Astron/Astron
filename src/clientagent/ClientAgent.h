#pragma once
#include "core/Role.h"
#include "Client.h"

#include <boost/asio.hpp>

// A ChannelTracker is used to keep track of available and allocated channels that
// the ClientAgent can use to assign to new Clients.
// TODO: Consider moving to util/ this class might be reusable in other roles that utilize ranges.
class ChannelTracker
{
	public:
		ChannelTracker(channel_t min, channel_t max);

		channel_t alloc_channel();
		void free_channel(channel_t channel);

	private:
		channel_t m_next;
		channel_t m_max;
		std::queue<channel_t> m_unused_channels;
};

class ClientAgent : public Role
{
		friend class Client;

	public:
		ClientAgent(RoleConfig rolconfig);
		~ClientAgent();

		// start_accept waits for a new client connection and calls handle_accept when received.
		void start_accept();

		// handle_accepts generates a new Client object from a connection, then calls start_accept.
		void handle_accept(boost::asio::ip::tcp::socket *socket, const boost::system::error_code &ec);

		// handle_datagram handles Datagrams received from the message director.
		// Currently the ClientAgent does not handle any datagrams,
		// and delegates everything to the Client objects.
		void handle_datagram(Datagram &in_dg, DatagramIterator &dgi);

		const std::string& get_version() const
		{
			return m_server_version;
		}

		const uint32_t get_hash() const
		{
			return m_hash;
		}

	private:
		boost::asio::ip::tcp::acceptor *m_acceptor;
		std::string m_client_type;
		std::string m_server_version;
		ChannelTracker m_ct;
		LogCategory *m_log;
		uint32_t m_hash;
};
