#include "core/global.h"
#include "NetworkClient.h"
#include <boost/bind.hpp>
#include <stdexcept>

using boost::asio::ip::tcp;

NetworkClient::NetworkClient() : m_socket(NULL), m_buffer(new uint8_t[2]),
	m_bytes_to_go(2), m_bufsize(2), m_is_data(false)
{
}

NetworkClient::NetworkClient(tcp::socket *socket) : m_socket(socket), m_buffer(new uint8_t[2]),
	m_bytes_to_go(2), m_bufsize(2), m_is_data(false)
{
	start_receive();
}

NetworkClient::~NetworkClient()
{
	m_socket->close();
	delete m_socket;
	delete m_buffer;
}

void NetworkClient::set_socket(tcp::socket *socket)
{
	if(m_socket)
	{
		throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
	}
	m_socket = socket;
	start_receive();
}

void NetworkClient::start_receive()
{
	uint16_t offset = m_bufsize - m_bytes_to_go;
	m_socket->async_receive(boost::asio::buffer(m_buffer + offset, m_bufsize - offset), boost::bind(&NetworkClient::read_handler,
	                        this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void NetworkClient::network_send(Datagram &dg)
{
	//TODO: make this asynch if necessary
	uint16_t len = dg.size();
	m_socket->send(boost::asio::buffer((uint8_t*)&len, 2));
	m_socket->send(boost::asio::buffer(dg.get_data(), dg.size()));
}

void NetworkClient::read_handler(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if(ec.value() != 0)
	{
		network_disconnect();
	}
	else
	{
		m_bytes_to_go -= bytes_transferred;
		if(m_bytes_to_go == 0)
		{
			if(!m_is_data)
			{
				m_bufsize = *(uint16_t*)m_buffer;
				delete [] m_buffer;
				m_buffer = new uint8_t[m_bufsize];
				m_bytes_to_go = m_bufsize;
				m_is_data = true;
			}
			else
			{
				Datagram dg(m_buffer, m_bufsize);
				delete [] m_buffer;//dg makes a copy
				m_bufsize = 2;
				m_buffer = new uint8_t[m_bufsize];
				m_bytes_to_go = m_bufsize;
				m_is_data = false;
				network_datagram(dg);
			}
		}
		start_receive();
	}
}
