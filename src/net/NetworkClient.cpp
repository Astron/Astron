#include "core/global.h"
#include "NetworkClient.h"
#include <boost/bind.hpp>
#include <stdexcept>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

NetworkClient::NetworkClient() : m_socket(nullptr), m_secure_socket(nullptr), m_ssl_enabled(false)
{
}

NetworkClient::NetworkClient(tcp::socket *socket) : m_socket(socket), m_secure_socket(nullptr),
    m_ssl_enabled(false)
{
    start_receive();
}

NetworkClient::NetworkClient(ssl::stream<tcp::socket>* stream) :
    m_socket(&stream->next_layer()), m_secure_socket(stream), m_ssl_enabled(true)
{
    start_receive();
}

NetworkClient::~NetworkClient()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_ssl_enabled) {
        // This also deletes m_socket:
        delete m_secure_socket;
    } else {
        // ONLY delete m_socket if we must do so directly. If it's wrapped in
        // an SSL stream, the stream "owns" the socket.
        delete m_socket;
    }
    delete [] m_data_buf;
}

void NetworkClient::set_socket(tcp::socket *socket)
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

    start_receive();
}

void NetworkClient::set_socket(ssl::stream<tcp::socket> *stream)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }

    m_ssl_enabled = true;
    m_secure_socket = stream;

    set_socket(&stream->next_layer());
}

void NetworkClient::start_receive()
{
    try {
        m_remote = m_socket->remote_endpoint();
    } catch(const boost::system::system_error&) {
        // A client might disconnect immediately after connecting.
        // Since we are in the constructor, ignore it. Resolves #122.
        // When the owner of the NetworkClient attempts to send or receive,
        // the error will occur and we'll cleanup then;
    }
    async_receive();
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
        send_disconnect(err.code());
    }
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    //TODO: make this asynch if necessary
    dgsize_t len = swap_le(dg->size());
    try {
        m_socket->non_blocking(true);
        m_socket->native_non_blocking(true);
        std::list<boost::asio::const_buffer> gather;
        gather.push_back(boost::asio::buffer((uint8_t*)&len, sizeof(dgsize_t)));
        gather.push_back(boost::asio::buffer(dg->get_data(), dg->size()));
        socket_write(gather);
    } catch(const boost::system::system_error &err) {
        // We assume that the message just got dropped if the remote end died
        // before we could send it.
        send_disconnect(err.code());
    }
}

void NetworkClient::send_disconnect()
{
    boost::system::error_code ec;
    send_disconnect(ec);
}

void NetworkClient::send_disconnect(const boost::system::error_code &ec)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    m_local_disconnect = true;
    m_disconnect_error = ec;
    m_socket->close();
}

void NetworkClient::handle_disconnect(const boost::system::error_code &ec)
{
    if(m_disconnect_handled) {
        return;
    }

    m_disconnect_handled = true;

    if(m_local_disconnect) {
        receive_disconnect(m_disconnect_error);
    } else {
        receive_disconnect(ec);
    }
}

void NetworkClient::receive_size(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec) {
        handle_disconnect(ec);
        return;
    }

    if(bytes_transferred != sizeof(m_size_buf)) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe, boost::system::system_category());
        handle_disconnect(epipe);
        return;
    }

    dgsize_t old_size = m_data_size;
    // required to disable strict-aliasing optimizations, which can break the code
    dgsize_t* new_size_p = (dgsize_t*)m_size_buf;
    m_data_size = swap_le(*new_size_p);
    if(m_data_size > old_size) {
        delete [] m_data_buf;
        m_data_buf = new uint8_t[m_data_size];
    }
    m_is_data = true;
    async_receive();
}

void NetworkClient::receive_data(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec) {
        handle_disconnect(ec);
        return;
    }

    if(bytes_transferred != m_data_size) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe, boost::system::system_category());
        handle_disconnect(epipe);
        return;
    }

    DatagramPtr dg = Datagram::create(m_data_buf, m_data_size); // Datagram makes a copy
    m_is_data = false;
    receive_datagram(dg);
    async_receive();
}

bool NetworkClient::is_connected()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    return m_socket->is_open();
}

void NetworkClient::socket_read(uint8_t* buf, size_t length, receive_handler_t callback)
{
    if(m_ssl_enabled) {
        async_read(*m_secure_socket, boost::asio::buffer(buf, length),
                   boost::bind(callback, this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    } else {
        async_read(*m_socket, boost::asio::buffer(buf, length),
                   boost::bind(callback, this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
}

void NetworkClient::socket_write(std::list<boost::asio::const_buffer>& buffers)
{
    if(m_ssl_enabled) {
        write(*m_secure_socket, buffers);
    } else {
        write(*m_socket, buffers);
    }
}
