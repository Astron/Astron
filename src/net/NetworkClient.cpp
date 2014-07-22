#include "NetworkClient.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include "core/global.h"
#include "config/ConfigVariable.h"
using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

static ConfigGroup nc_configgroup("networkclient");
static ConfigVariable<unsigned int> max_queue_size("max_write_queue_size", 256*1024, nc_configgroup);
static ConfigVariable<unsigned int> write_send_timeout("write_send_timeout", 5000, nc_configgroup);

NetworkClient::NetworkClient() : m_socket(nullptr), m_secure_socket(nullptr), m_remote(),
	m_async_timer(io_service), m_ssl_enabled(false), m_is_sending(false), m_is_receiving(false),
	m_is_data(false), m_data_buf(nullptr), m_data_size(0), m_total_queue_size(0), m_send_queue()
{
}

NetworkClient::NetworkClient(tcp::socket *socket) : m_socket(socket), m_secure_socket(nullptr),
	m_remote(), m_async_timer(io_service), m_ssl_enabled(false), m_is_sending(false),
	m_is_receiving(false), m_is_data(false), m_data_buf(nullptr), m_data_size(0),
	m_total_queue_size(0), m_send_queue()
{
	start_receive();
}

NetworkClient::NetworkClient(ssl::stream<tcp::socket>* stream) :
    m_socket(&stream->next_layer()), m_secure_socket(stream), m_remote(), m_async_timer(io_service),
    m_ssl_enabled(true), m_is_sending(false), m_is_receiving(false), m_is_data(false),
    m_data_buf(nullptr), m_data_size(0), m_total_queue_size(0), m_send_queue()
{
	start_receive();
}

NetworkClient::~NetworkClient()
{
	if(m_socket)
	{
		lock_guard<recursive_mutex> lock(m_lock);
		m_socket->close();
	}
	delete m_socket;
	delete [] m_data_buf;
}

void NetworkClient::set_socket(tcp::socket *socket)
{
	lock_guard<recursive_mutex> lock(m_lock);
	if(m_socket)
	{
		throw logic_error("Trying to set a socket of a network client whose socket was already set.");
	}
	m_socket = socket;

	boost::asio::socket_base::keep_alive keepalive(true);
	m_socket->set_option(keepalive);

	boost::asio::ip::tcp::no_delay nodelay(true);
	m_socket->set_option(nodelay);

	start_receive();
}

void NetworkClient::set_socket(ssl::stream<tcp::socket> *stream)
{
	lock_guard<recursive_mutex> lock(m_lock);
	if(m_socket)
	{
		throw logic_error("Trying to set a socket of a network client whose socket was already set.");
	}

	m_ssl_enabled = true;
	m_secure_socket = stream;

	set_socket(&stream->next_layer());
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
	lock_guard<recursive_mutex> lock(m_lock);

	if(m_is_sending)
	{
		m_send_queue.push(dg);
		m_total_queue_size += dg->size();
		if(m_total_queue_size > max_queue_size.get_val())
		{
			send_disconnect();
			return;
		}
	}
	else
	{
		m_is_sending = true;
		async_send(dg);
	}
}

void NetworkClient::send_disconnect()
{
	lock_guard<recursive_mutex> lock(m_lock);
	if(m_socket->is_open())
	{
		m_socket->close();
	}
}

bool NetworkClient::is_connected()
{
	lock_guard<recursive_mutex> lock(m_lock);
	return m_socket->is_open();
}

void NetworkClient::start_receive()
{
	try
	{
		m_remote = m_socket->remote_endpoint();
	}
	catch (const boost::system::system_error&)
	{
		// A client might disconnect immediately after connecting.
		// Since we are in the constructor, ignore it. Resolves #122.
		// When the owner of the NetworkClient attempts to send or receive,
		// the error will occur and we'll cleanup then;
	}
	async_receive();
}

void NetworkClient::async_receive()
{
	lock_guard<recursive_mutex> lock(m_lock);
	try
	{
		m_is_receiving = true;
		if(m_is_data) // Read data
		{
			socket_read(m_data_buf, m_data_size, &NetworkClient::receive_data);
		}
		else // Read length
		{
			socket_read(m_size_buf, sizeof(dgsize_t), &NetworkClient::receive_size);
		}
	}
	catch(const boost::system::system_error& err)
	{
		m_is_receiving = false;

		// An exception happening when trying to initiate a read is a clear
		// indicator that something happened to the connection, therefore:
		send_disconnect();
	}
}

void NetworkClient::receive_size(const boost::system::error_code &ec, size_t /*bytes_transferred*/)
{
	m_is_receiving = false;

	if(ec.value() != 0)
	{
		lock_guard<recursive_mutex> lock(m_lock);

		// If waiting for asio send callback, let send handle the disconnect.
		if(!m_is_sending)
		{
			receive_disconnect();
		}

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
	m_is_receiving = false;

	if(ec.value() != 0)
	{
		lock_guard<recursive_mutex> lock(m_lock);

		// If waiting for asio send callback, let send handle the disconnect.
		if(!m_is_sending)
		{
			receive_disconnect();
		}

		return;
	}

	DatagramPtr dg = Datagram::create(m_data_buf, m_data_size); // Datagram makes a copy
	m_is_data = false;
	receive_datagram(dg);
	async_receive();
}

void NetworkClient::async_send(DatagramHandle dg)
{
	lock_guard<recursive_mutex> lock(m_lock);
	try
	{
		dgsize_t len = swap_le(dg->size());
		list<boost::asio::const_buffer> gather;
		gather.push_back(boost::asio::buffer((uint8_t*)&len, sizeof(dgsize_t)));
		gather.push_back(boost::asio::buffer(dg->get_data(), dg->size()));
		socket_write(gather);
	}
	catch(const boost::system::system_error& err)
	{
		// An exception happening when trying to initiate a send is a clear
		// indicator that something happened to the connection, therefore:
		send_disconnect();
	}
}

void NetworkClient::send_finished(const boost::system::error_code &ec, size_t /*bytes_transferred*/)
{
	lock_guard<recursive_mutex> lock(m_lock);

	// Cancel the outstanding timeout
	m_async_timer.cancel();

	// Check if the write had errors
	if(ec.value() != 0)
	{
		m_is_sending = false;

		// If waiting for asio receive callback, let receive handle the disconnect.
		if(!m_is_receiving)
		{
			receive_disconnect();
		}

		return;
	}

	// Check if we have more items in the queue
	if(m_send_queue.size() > 0)
	{
		// Send the next item in the queue
		DatagramHandle dg = m_send_queue.front();
		m_total_queue_size -= dg->size();
		m_send_queue.pop();
		async_send(dg);
		return;
	}

	// Nothing left in the queue to send, lets open up for another write
	m_is_sending = false;
}

void NetworkClient::send_expired(const boost::system::error_code& ec)
{
	// operation_aborted is received if the the timer is cancelled,
	//     ie. if the send completed before it expires, so don't do anything
	if(ec != boost::asio::error::operation_aborted)
	{
		send_disconnect();
	}
}

void NetworkClient::async_cancel()
{
	lock_guard<recursive_mutex> lock(m_lock);
	try
	{
		m_async_timer.cancel();
		m_socket->cancel();
	}
	catch(const boost::system::system_error&)
	{
		// Ignore errors attempting to cleanup
	}
}

void NetworkClient::socket_read(uint8_t* buf, size_t length, receive_handler_t callback)
{
	if(m_ssl_enabled)
	{
		async_read(*m_secure_socket, boost::asio::buffer(buf, length),
		           boost::bind(callback, this,
		           boost::asio::placeholders::error,
		           boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		async_read(*m_socket, boost::asio::buffer(buf, length),
		           boost::bind(callback, this,
		           boost::asio::placeholders::error,
		           boost::asio::placeholders::bytes_transferred));
	}
}

void NetworkClient::socket_write(list<boost::asio::const_buffer>& buffers)
{
	// Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
	if(write_send_timeout.get_val() > 0)
	{
		m_async_timer.expires_from_now(boost::posix_time::milliseconds(write_send_timeout.get_val()));
		m_async_timer.async_wait(boost::bind(&NetworkClient::send_expired, this,
		                                     boost::asio::placeholders::error));
	}

	// Start async write
	if(m_ssl_enabled)
	{
		async_write(*m_secure_socket, buffers,
		            boost::bind(&NetworkClient::send_finished, this,
		            boost::asio::placeholders::error,
		            boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		async_write(*m_socket, buffers,
		            boost::bind(&NetworkClient::send_finished, this,
		            boost::asio::placeholders::error,
		            boost::asio::placeholders::bytes_transferred));
	}
}
