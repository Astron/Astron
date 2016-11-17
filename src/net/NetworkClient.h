#pragma once
#include <list>
#include <queue>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "util/Datagram.h"

class NetworkClient;

class NetworkHandler
{
  protected:
    // receive_datagram is called when both a datagram's size and its data
    //     have been received asynchronously from the network.
    virtual void receive_datagram(DatagramHandle dg) = 0;
    // receive_disconnect is called when the remote host closes the
    //     connection or otherwise when the tcp connection is lost.
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
    void set_write_timeout(unsigned int timeout);
    void set_write_buffer(uint64_t max_bytes);

    // send_datagram immediately sends the datagram over TCP (blocking).
    void send_datagram(DatagramHandle dg);
    // disconnect closes the TCP connection
    void disconnect();
    void disconnect(const boost::system::error_code &ec);
    // is_connected returns true if the TCP connection is active, or false otherwise
    bool is_connected();

    inline boost::asio::ip::tcp::endpoint get_remote()
    {
        return m_remote;
    }
    inline boost::asio::ip::tcp::endpoint get_local()
    {
        return m_local;
    }

  private:
    /* Asynchronous call loop */
    // async_send is called by send_datagram or send_finished when the socket is available
    //     for writing to send the next datagram in the queue.
    void async_send(DatagramHandle dg);
    // send_finished is called when an async_send has completed
    void send_finished(const boost::system::error_code &ec);
    // send_expired is called when an async_send has expired
    void send_expired(const boost::system::error_code& error);

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

    typedef void (NetworkClient::*receive_handler_t)(const boost::system::error_code&, size_t);

    void socket_read(uint8_t* buf, size_t length, receive_handler_t callback);
    void socket_write(const uint8_t* buf, size_t length);

    void handle_disconnect(const boost::system::error_code &ec);

    bool m_is_sending = false;
    uint8_t *m_send_buf = nullptr;

    bool m_is_data = false;

    NetworkHandler *m_handler;
    boost::asio::ip::tcp::socket *m_socket;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> *m_secure_socket;
    boost::asio::ip::tcp::endpoint m_remote;
    boost::asio::ip::tcp::endpoint m_local;
    boost::asio::deadline_timer m_async_timer;
    uint8_t m_size_buf[sizeof(dgsize_t)];
    uint8_t* m_data_buf = nullptr;
    dgsize_t m_data_size = 0;

    uint64_t m_total_queue_size = 0;
    uint64_t m_max_queue_size = 0;
    unsigned int m_write_timeout = 0;
    std::queue<DatagramHandle> m_send_queue;

    std::recursive_mutex m_lock;

    bool m_disconnect_handled = false;
    bool m_local_disconnect = false;
    boost::system::error_code m_disconnect_error;
};
