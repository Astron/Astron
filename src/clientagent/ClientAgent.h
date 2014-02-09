#pragma once
#include "core/Role.h"
#include "Client.h"

#include <boost/asio.hpp>

extern ConfigGroup ca_client_config;
extern ConfigVariable<std::string> ca_client_type;

// A ChannelTracker is used to keep track of available and allocated channels that
// the ClientAgent can use to assign to new Clients.
// TODO: Consider moving to util/ this class might be reusable in other roles that utilize ranges.
class ChannelTracker
{
	public:
		ChannelTracker(channel_t min = INVALID_CHANNEL, channel_t max = INVALID_CHANNEL);

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

		const std::string& get_version()
		{
			return m_server_version;
		}

	private:
		boost::asio::ip::tcp::acceptor *m_acceptor;
		std::string m_client_type;
		std::string m_server_version;
		ChannelTracker m_ct;
		ConfigNode m_clientconfig;
		LogCategory *m_log;
};
