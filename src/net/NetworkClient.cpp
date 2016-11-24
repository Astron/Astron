#include "NetworkClient.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include "core/global.h"
#include "config/ConfigVariable.h"
using namespace std;
using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

NetworkClient::NetworkClient(NetworkHandler *handler) : m_handler(handler), m_socket(nullptr),
    m_secure_socket(nullptr),
    m_async_timer(io_service), m_send_queue()
{
}

NetworkClient::~NetworkClient()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    assert(!is_connected());
    if(m_secure_socket) {
        // This also deletes m_socket:
        delete m_secure_socket;
    } else {
        // ONLY delete m_socket if we must do so directly. If it's wrapped in
        // an SSL stream, the stream "owns" the socket.
        delete m_socket;
    }

    delete [] m_data_buf;
    delete [] m_send_buf;
}

void NetworkClient::initialize(tcp::socket *socket)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }
    m_socket = socket;

    boost::asio::socket_base::keep_alive keepalive(true);
    m_socket->set_option(keepalive);

    boost::asio::ip::tcp::no_delay nodelay(true);
    m_socket->set_option(nodelay);

    bool endpoints_set = (m_remote.port() && m_local.port());
    if(endpoints_set || determine_endpoints(m_remote, m_local)) {
        async_receive();
    }
}

void NetworkClient::initialize(tcp::socket *socket,
                               const tcp::endpoint &remote,
                               const tcp::endpoint &local)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }
    m_remote = remote;
    m_local = local;

    initialize(socket);
}

void NetworkClient::initialize(ssl::stream<tcp::socket> *stream)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }

    m_secure_socket = stream;

    initialize(&stream->next_layer());
}

void NetworkClient::initialize(ssl::stream<tcp::socket> *stream,
                               const tcp::endpoint &remote,
                               const tcp::endpoint &local)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }

    m_secure_socket = stream;

    initialize(&stream->next_layer(), remote, local);
}

bool NetworkClient::determine_endpoints(tcp::endpoint &remote, tcp::endpoint &local)
{
    boost::system::error_code ec;
    remote = m_socket->remote_endpoint(ec);
    local = m_socket->local_endpoint(ec);

    if(ec) {
        disconnect(ec);
        return false;
    } else {
        return true;
    }
}

void NetworkClient::set_write_timeout(unsigned int timeout)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_write_timeout = timeout;
}

void NetworkClient::set_write_buffer(uint64_t max_bytes)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_max_queue_size = max_bytes;
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
    lock_guard<recursive_mutex> lock(m_lock);

    if(m_is_sending) {
        m_send_queue.push(dg);
        m_total_queue_size += dg->size();
        if(m_total_queue_size > m_max_queue_size && m_max_queue_size != 0) {
            boost::system::error_code enobufs(boost::system::errc::errc_t::no_buffer_space,
                                              boost::system::system_category());
            disconnect(enobufs);
        }
    } else {
        m_is_sending = true;
        async_send(dg);
    }
}

bool NetworkClient::is_connected()
{
    lock_guard<recursive_mutex> lock(m_lock);
    return m_socket->is_open();
}

void NetworkClient::async_receive()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    try {
        if(m_is_data) { // Read data
            socket_read(m_data_buf, m_data_size, &NetworkClient::receive_data);
        } else { // Read length
            socket_read(m_size_buf, sizeof(dgsize_t), &NetworkClient::receive_size);
        }
    } catch(const boost::system::system_error &err) {
        // An exception happening when trying to initiate a read is a clear
        // indicator that something happened to the connection. Therefore:
        disconnect(err.code());
    }
}

void NetworkClient::disconnect()
{
    boost::system::error_code ec;
    disconnect(ec);
}

void NetworkClient::disconnect(const boost::system::error_code &ec)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if(m_local_disconnect || m_disconnect_handled) {
        // We've already set the error code and closed the socket; wait.
        return;
    }

    m_local_disconnect = true;
    m_disconnect_error = ec;

    m_socket->cancel();
    m_socket->close();

    m_async_timer.cancel();
}

void NetworkClient::handle_disconnect(const boost::system::error_code &ec)
{
    // Lock not needed: This is only called internally, from a function that
    // already holds the lock.

    if(m_disconnect_handled) {
        return;
    }
    m_disconnect_handled = true;

    if(is_connected()) {
        m_socket->close();
    }

    if(m_local_disconnect) {
        m_handler->receive_disconnect(m_disconnect_error);
    } else {
        m_handler->receive_disconnect(ec);
    }
}

void NetworkClient::receive_size(const boost::system::error_code &ec, size_t bytes_transferred)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    if(ec) {
        handle_disconnect(ec);
        return;
    }

    if(bytes_transferred != sizeof(m_size_buf)) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe,
                                        boost::system::system_category());
        handle_disconnect(epipe);
        return;
    }

    // required to disable strict-aliasing optimizations, which can break the code
    dgsize_t* new_size_p = (dgsize_t*)m_size_buf;
    m_data_size = swap_le(*new_size_p);
    m_data_buf = new uint8_t[m_data_size];
    m_is_data = true;
    async_receive();
}

void NetworkClient::receive_data(const boost::system::error_code &ec, size_t bytes_transferred)
{
    DatagramPtr dg;

    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);

        if(ec) {
            handle_disconnect(ec);
            return;
        }

        if(bytes_transferred != m_data_size) {
            boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe,
                                            boost::system::system_category());
            handle_disconnect(epipe);
            return;
        }

        dg = Datagram::create(m_data_buf, m_data_size); // Datagram makes a copy

        delete [] m_data_buf;
        m_data_buf = nullptr;

        m_is_data = false;
        async_receive();
    }

    // Do NOT hold the lock when calling this. Our subclass may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    m_handler->receive_datagram(dg);
}

void NetworkClient::async_send(DatagramHandle dg)
{
    lock_guard<recursive_mutex> lock(m_lock);

    size_t buffer_size = sizeof(dgsize_t) + dg->size();
    dgsize_t len = swap_le(dg->size());
    m_send_buf = new uint8_t[buffer_size];
    memcpy(m_send_buf, (uint8_t*)&len, sizeof(dgsize_t));
    memcpy(m_send_buf + sizeof(dgsize_t), dg->get_data(), dg->size());

    try {
        socket_write(m_send_buf, buffer_size);
    } catch(const boost::system::system_error& err) {
        // An exception happening when trying to initiate a send is a clear
        // indicator that something happened to the connection, therefore:
        disconnect(err.code());
    }
}

void NetworkClient::send_finished(const boost::system::error_code &ec)
{
    lock_guard<recursive_mutex> lock(m_lock);

    // Cancel the outstanding timeout
    m_async_timer.cancel();

    // Discard the buffer we just used:
    delete [] m_send_buf;
    m_send_buf = nullptr;

    // Check if the write had errors
    if(ec.value() != 0) {
        m_is_sending = false;
        disconnect(ec);
        return;
    }

    // Check if we have more items in the queue
    if(m_send_queue.size() > 0) {
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
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    // operation_aborted is received if the the timer is cancelled,
    //     ie. if the send completed before it expires, so don't do anything
    if(ec != boost::asio::error::operation_aborted) {
        boost::system::error_code etimeout(boost::system::errc::errc_t::timed_out,
                                           boost::system::system_category());
        disconnect(etimeout);
    }
}

void NetworkClient::socket_read(uint8_t* buf, size_t length, receive_handler_t callback)
{
    // Lock not needed: This is only called internally, from a function that
    // already holds the lock.

    if(m_secure_socket) {
        async_read(*m_secure_socket, boost::asio::buffer(buf, length),
                   boost::bind(callback, shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    } else {
        async_read(*m_socket, boost::asio::buffer(buf, length),
                   boost::bind(callback, shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
}

void NetworkClient::socket_write(const uint8_t* buf, size_t length)
{
    // Lock not needed: This is only called internally, from a function that
    // already holds the lock.

    // Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
    if(m_write_timeout > 0) {
        m_async_timer.expires_from_now(boost::posix_time::milliseconds(m_write_timeout));
        m_async_timer.async_wait(boost::bind(&NetworkClient::send_expired, shared_from_this(),
                                             boost::asio::placeholders::error));
    }

    // Start async write
    if(m_secure_socket) {
        async_write(*m_secure_socket, boost::asio::buffer(buf, length),
                    boost::bind(&NetworkClient::send_finished, shared_from_this(),
                                boost::asio::placeholders::error));
    } else {
        async_write(*m_socket, boost::asio::buffer(buf, length),
                    boost::bind(&NetworkClient::send_finished, shared_from_this(),
                                boost::asio::placeholders::error));
    }
}
