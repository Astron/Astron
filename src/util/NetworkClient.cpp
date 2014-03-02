#include "core/global.h"
#include "NetworkClient.h"
#include <boost/bind.hpp>
#include <stdexcept>

using boost::asio::ip::tcp;

NetworkClient::NetworkClient() : m_socket(NULL), m_data_buf(NULL), m_data_size(0), m_is_data(false)
{
}

NetworkClient::NetworkClient(tcp::socket *socket) : m_socket(socket), m_data_buf(NULL),
	m_data_size(0), m_is_data(false)
{
	start_receive();
}

NetworkClient::~NetworkClient()
{
	if(m_socket)
	{
		m_socket->close();
	}
	delete m_socket;
	delete [] m_data_buf;
}

void NetworkClient::set_socket(tcp::socket *socket)
{
	if(m_socket)
	{
		throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
	}
	m_socket = socket;

	boost::asio::socket_base::keep_alive keepalive(true);
	m_socket->set_option(keepalive);

	boost::asio::ip::tcp::no_delay nodelay(true);
	m_socket->set_option(nodelay);

	start_receive();
}

void NetworkClient::start_receive()
{
	async_receive();
}

void NetworkClient::async_receive()
{
	try
	{
		if(m_is_data) // Read data
		{
			async_read(*m_socket, boost::asio::buffer(m_data_buf, m_data_size),
			           boost::bind(&NetworkClient::receive_data, this,
			           boost::asio::placeholders::error,
			           boost::asio::placeholders::bytes_transferred));
		}
		else // Read length
		{
			async_read(*m_socket, boost::asio::buffer(m_size_buf, sizeof(dgsize_t)),
			           boost::bind(&NetworkClient::receive_size, this,
			           boost::asio::placeholders::error,
			           boost::asio::placeholders::bytes_transferred));
		}
	}
	catch(std::exception&)
	{
		// An exception happening when trying to initiate a read is a clear
		// indicator that something happened to the connection. Therefore:
		send_disconnect();
	}
}

void NetworkClient::send_datagram(Datagram &dg)
{
	//TODO: make this asynch if necessary
	dgsize_t len = swap_le(dg.size());
	try
	{
		m_socket->non_blocking(true);
		m_socket->native_non_blocking(true);
		std::list<boost::asio::const_buffer> gather;
		gather.push_back(boost::asio::buffer((uint8_t*)&len, sizeof(dgsize_t)));
		gather.push_back(boost::asio::buffer(dg.get_data(), dg.size()));
		m_socket->send(gather);
	}
	catch(std::exception&)
	{
		// We assume that the message just got dropped if the remote end died
		// before we could send it.
		send_disconnect();
	}
}

void NetworkClient::send_disconnect()
{
	m_socket->close();
}

void NetworkClient::receive_size(const boost::system::error_code &ec, size_t /*bytes_transferred*/)
{
	// TODO: We might want to actually check here that bytes_transferred is the expected value

	if(ec.value() != 0)
	{
		receive_disconnect();
		return;
	}

	dgsize_t old_size = m_data_size;
	// required to disable strict-aliasing optimizations, which can break the code
	dgsize_t* new_size_p = (dgsize_t*)m_size_buf;
	m_data_size = swap_le(*new_size_p);
	if(m_data_size > old_size)
	{
		delete [] m_data_buf;
		m_data_buf = new uint8_t[m_data_size];
	}
	m_is_data = true;
	async_receive();
}

void NetworkClient::receive_data(const boost::system::error_code &ec, size_t /*bytes_transferred*/)
{
	// TODO: We might want to actually check here that bytes_transferred is the expected value

	if(ec.value() != 0)
	{
		receive_disconnect();
		return;
	}

	Datagram dg(m_data_buf, m_data_size); // Datagram makes a copy
	m_is_data = false;
	receive_datagram(dg);
	async_receive();
}

bool NetworkClient::is_connected()
{
	return m_socket->is_open();
}
