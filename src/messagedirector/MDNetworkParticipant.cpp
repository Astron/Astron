#include "MDNetworkParticipant.h"
#include "core/global.h"
#include <boost/bind.hpp>


MDNetworkParticipant::MDNetworkParticipant(boost::asio::ip::tcp::socket *socket)
	: MDParticipantInterface(), m_socket(socket), m_buffer(new char[2]), m_bytes_to_go(2),
	m_bufsize(2), m_is_data(false)
{
	start_receive();
}

MDNetworkParticipant::~MDNetworkParticipant()
{
	m_socket->close();
	delete m_buffer;
}

bool MDNetworkParticipant::handle_datagram(Datagram *dg, DatagramIterator *dgi)
{
	//TODO: make this asynch
	m_socket->send(boost::asio::buffer(dg->get_data(), dg->get_buf_end());
	return true;
}

void MDNetworkParticipant::start_receive()
{
	unsigned short offset = m_bufsize - m_bytes_to_go;
	m_socket->async_receive(boost::asio::buffer(m_buffer+offset, m_bufsize-offset), boost::bind(&MDNetworkParticipant::read_handler,
		this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void MDNetworkParticipant::read_handler(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if(ec.value() != 0)
	{
		gLogger->debug() << "Dropping client because of error: " << ec.category().message(ec.value()) << std::endl;
		delete this;
	}
	else
	{
		m_bytes_to_go -= bytes_transferred;
		if(m_bytes_to_go == 0)
		{
			if(!m_is_data)
			{
				m_bufsize = *(unsigned short*)m_buffer;
				delete [] m_buffer;
				m_buffer = new char[m_bufsize];
				m_bytes_to_go = m_bufsize;
				m_is_data = true;
			}
			else
			{
				Datagram dg(std::string(m_buffer, m_bufsize));
				delete [] m_buffer;//dg makes a copy
				m_bufsize = 2;
				m_buffer = new char[m_bufsize];
				m_bytes_to_go = m_bufsize;
				//TODO: implement below
				//MessageDirector::singleton.route_datagram(dg);
			}
		}

		start_receive();
	}
}