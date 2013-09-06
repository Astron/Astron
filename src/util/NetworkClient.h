#pragma once
#include <boost/asio.hpp>
#include "Datagram.h"

class NetworkClient
{
	protected:
		NetworkClient(boost::asio::ip::tcp::socket *socket);
		virtual ~NetworkClient();

		virtual void network_datagram(Datagram &dg) = 0;
		virtual void network_disconnect() = 0;
		void network_send(Datagram *dg);
	private:
		void start_receive();
		void read_handler(const boost::system::error_code &ec, size_t bytes_transferred);
		boost::asio::ip::tcp::socket *m_socket;

		char* m_buffer;
		short m_bytes_to_go;
		short m_bufsize;
		bool m_is_data;
};