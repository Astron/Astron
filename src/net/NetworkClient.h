#pragma once
#include <list>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "util/Datagram.h"

class NetworkClient
{
  public:
    // send_datagram immediately sends the datagram over TCP (blocking).
    virtual void send_datagram(DatagramHandle dg);
    // send_disconnect closes the TCP connection
    virtual void send_disconnect();
    // is_connected returns true if the TCP connection is active, or false otherwise
    bool is_connected();

  protected:
    NetworkClient();
    NetworkClient(boost::asio::ip::tcp::socket *socket);
    NetworkClient(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream);
    virtual ~NetworkClient();
    void set_socket(boost::asio::ip::tcp::socket *socket);
    void set_socket(boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *stream);


    /** Pure virtual methods **/

    // receive_datagram is called when both a datagram's size and its data
    //     have been received asynchronously from the network.
    virtual void receive_datagram(DatagramHandle dg) = 0;
    // receive_disconnect is called when the remote host closes the
    //     connection or otherwise when the tcp connection is lost.
    virtual void receive_disconnect() = 0;


    /* Asynchronous call loop */

    // start_receive is called by the constructor or set_socket
    //     after m_socket is set to a provided socket.
    virtual void start_receive();

    // async_receive is called by start_receive to begin receiving data, then by receive_size
    //     or receive_data to wait for the next set of data.
    virtual void async_receive();

    // receive_size is called by async_receive when receiving the datagram size
    virtual void receive_size(const boost::system::error_code &ec, size_t bytes_transferred);
    // receive_data is called by async_receive when receiving the datagram data
    virtual void receive_data(const boost::system::error_code &ec, size_t bytes_transferred);


    boost::asio::ip::tcp::socket *m_socket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *m_secure_socket;
    boost::asio::ip::tcp::endpoint m_remote;

  private:
    typedef void (NetworkClient::*receive_handler_t)(const boost::system::error_code&, size_t);

    void socket_read(uint8_t* buf, size_t length, receive_handler_t callback);
    void socket_write(std::list<boost::asio::const_buffer>&);

    bool m_ssl_enabled;
    uint8_t m_size_buf[sizeof(dgsize_t)];
    uint8_t* m_data_buf;
    dgsize_t m_data_size;
    bool m_is_data;

    std::recursive_mutex m_lock;
};
