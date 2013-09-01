#pragma once
#include "MessageDirector.h"
#include <vector>

class MDNetworkParticipant : public MDParticipantInterface
{
	public:
		MDNetworkParticipant(boost::asio::ip::tcp::socket *socket);
		virtual ~MDNetworkParticipant();
		virtual bool handle_datagram(Datagram *dg, DatagramIterator &dgi);
	private:
		void start_receive();
		void read_handler(const boost::system::error_code &ec, size_t bytes_transferred);
		boost::asio::ip::tcp::socket *m_socket;

		char* m_buffer;
		short m_bytes_to_go;
		short m_bufsize;
		bool m_is_data;
};
