#include "core/global.h"
#include "NetworkClient.h"
#include <boost/bind.hpp>
#include <stdexcept>
#include <config/ConfigVariable.h>

using boost::asio::ip::tcp;

static ConfigGroup nc_configgroup("networkclient");
static ConfigVariable<unsigned int> max_queue_size("max_write_queue_size", 256*1024, nc_configgroup);
static ConfigVariable<unsigned int> write_send_timeout("write_send_timeout", 5, nc_configgroup);

class NetworkWriteOperation
{
	public:
		NetworkWriteOperation(NetworkClient *nc) : m_network_client(nc)
		{
			std::list<boost::asio::const_buffer> gather;
			nc->m_send_in_progress = true;
			m_sending_handles = new DatagramHandle[nc->m_send_queue.size()];
			m_dg_sizes = new dgsize_t[nc->m_send_queue.size()];
			unsigned int i = 0;
			while(!nc->m_send_queue.empty())
			{
				DatagramHandle dg = nc->m_send_queue.front();
				m_sending_handles[i] = dg;
				nc->m_send_queue.pop();
				m_dg_sizes[i] = swap_le(dg->size());
				gather.push_back(boost::asio::buffer((uint8_t*)(m_dg_sizes+i), sizeof(dgsize_t)));
				gather.push_back(boost::asio::buffer(dg->get_data(), dg->size()));
				++i;
			}
			async_write(*nc->m_socket, gather, boost::bind(&NetworkWriteOperation::write_handler,
				this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}

		NetworkWriteOperation::~NetworkWriteOperation()
		{
			delete [] m_sending_handles;
			delete [] m_dg_sizes;
		}

		void write_handler(const boost::system::error_code &ec, size_t bytes_transferred)
		{
			if(m_network_client)
			{
				m_network_client->async_write_done(ec.value() == 0);
			}
			delete this;
		}

		void network_client_deleted()
		{
			m_network_client = NULL;
		}

	private:
		DatagramHandle *m_sending_handles;
		dgsize_t *m_dg_sizes;
		NetworkClient *m_network_client;
};

NetworkClient::NetworkClient() : m_socket(NULL), 
	m_data_buf(NULL), m_data_size(0), m_is_data(false), m_async_timer(io_service),
	m_pending_drop(false)
{
}

NetworkClient::NetworkClient(tcp::socket *socket) : m_socket(socket), m_data_buf(NULL),
	m_data_size(0), m_is_data(false), m_send_in_progress(false), m_send_queue(), 
	m_netwriteop(NULL), m_send_queue_size(0), m_async_timer(io_service),
	m_pending_drop(false)
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
	if(m_netwriteop)
	{
		m_netwriteop->network_client_deleted();
	}
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

void NetworkClient::send_datagram(DatagramHandle dg)
{
	m_send_queue.push(dg);
	m_send_queue_size += dg->size();
	if(m_send_queue_size > max_queue_size.get_val())
	{
		send_disconnect();
		return;
	}
	if(!m_send_in_progress)
	{
		m_async_timer.expires_from_now(boost::posix_time::seconds(write_send_timeout.get_val()));
		m_async_timer.async_wait(boost::bind(&NetworkClient::async_time_expired, this, 
			boost::asio::placeholders::error));
		make_network_write_op();
	}
}

void NetworkClient::send_disconnect()
{
	// The client is being disconnected, but we attempt to send the datagrams in queue
	// This should fix issue #126

	// These are the possible scenarios:

	// I. There's nothing being sent: just close the socket
	if(!m_send_in_progress)
	{
		m_socket->close();
	}

	// II. There's data being sent: tag it to be closed by async_write_done later
	else
	{
		m_pending_drop = true;
	}
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

	DatagramPtr dg = Datagram::create(m_data_buf, m_data_size); // Datagram makes a copy
	m_is_data = false;
	receive_datagram(dg);
	async_receive();
}

void NetworkClient::make_network_write_op()
{
	m_send_in_progress = true;
	m_netwriteop = new NetworkWriteOperation(this);
	m_send_queue_size = 0;
}

bool NetworkClient::is_connected()
{
	return m_socket->is_open();
}

void NetworkClient::async_write_done(bool success)
{
	m_netwriteop = NULL;
	if(!success)
	{
		send_disconnect();
	}
	else if(m_pending_drop)
	{
		m_socket->close();
	}
	else if(m_send_queue.empty())
	{
		m_send_in_progress = false;
		m_async_timer.cancel();
	}
	else
	{
		make_network_write_op();
	}
}

void NetworkClient::async_time_expired(const boost::system::error_code& error)
{
	if(error != boost::asio::error::operation_aborted)
	{
		send_disconnect();
	}
}