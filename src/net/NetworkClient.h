#pragma once
#include <list>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "util/Datagram.h"

// NOTES:
//
// Do not subclass NetworkClient. Instead, you should implement NetworkHandler
// and instantiate NetworkClient with std::make_shared.
//
// To begin receiving, pass it an ASIO socket or SSL stream via set_socket.
//
// You must not destruct your NetworkHandler implementor until
// receive_disconnect is called!

class NetworkClient;

class NetworkHandler
{
  protected:
    // receive_datagram is called when both a datagram's size and its data
    //     have been received asynchronously from the network.
    virtual void receive_datagram(DatagramHandle dg) = 0;
    // receive_disconnect is called when the remote host closes the
    //     connection or otherwise when the tcp connection is lost.
    //
    // NOTE: Your handler pointer must remain valid until this function is
    //     called, indicating that the NetworkClient has cleaned up.
    virtual void receive_disconnect(const boost::system::error_code &ec) = 0;

    friend class NetworkClient;
};

class NetworkClient : public std::enable_shared_from_this<NetworkClient>
{
  public:
    NetworkClient(NetworkHandler *handler);
    ~NetworkClient();

    void set_socket(boost::asio::ip::tcp::socket *socket);
    void set_socket(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream);

    // send_datagram immediately sends the datagram over TCP (blocking).
    void send_datagram(DatagramHandle dg);
    // disconnect closes the TCP connection
    void disconnect();
    void disconnect(const boost::system::error_code &ec);
    // is_connected returns true if the TCP connection is active, or false otherwise
    bool is_connected();

    inline boost::asio::ip::tcp::endpoint get_remote() { return m_remote; }
    inline boost::asio::ip::tcp::endpoint get_local() { return m_local; }

  private:
    /* Asynchronous call loop */

    // start_receive is called by the constructor or set_socket
    //     after m_socket is set to a provided socket.
    void start_receive();

    // async_receive is called by start_receive to begin receiving data, then by receive_size
    //     or receive_data to wait for the next set of data.
    void async_receive();

    // receive_size is called by async_receive when receiving the datagram size
    void receive_size(const boost::system::error_code &ec, size_t bytes_transferred);
    // receive_data is called by async_receive when receiving the datagram data
    void receive_data(const boost::system::error_code &ec, size_t bytes_transferred);


    boost::asio::ip::tcp::socket *m_socket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *m_secure_socket;

    typedef void (NetworkClient::*receive_handler_t)(const boost::system::error_code&, size_t);

    void socket_read(uint8_t* buf, size_t length, receive_handler_t callback);
    void socket_write(std::list<boost::asio::const_buffer>&);

    void handle_disconnect(const boost::system::error_code &ec);

    NetworkHandler *m_handler;
    boost::asio::ip::tcp::endpoint m_remote;
    boost::asio::ip::tcp::endpoint m_local;
    bool m_ssl_enabled;
    bool m_disconnect_handled = false;
    bool m_local_disconnect = false;
    boost::system::error_code m_disconnect_error;
    uint8_t m_size_buf[sizeof(dgsize_t)];
    uint8_t* m_data_buf = nullptr;
    dgsize_t m_data_size = 0;
    bool m_is_data = false;

    std::recursive_mutex m_lock;
};
