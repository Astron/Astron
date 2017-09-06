#pragma once
#include <list>
#include <queue>
#include <mutex>
#include <uvw.hpp>
#include "SocketWrapper.h"
#include "util/Datagram.h"

// NOTES:
//
// Do not subclass NetworkClient. Instead, you should implement NetworkHandler
// and instantiate NetworkClient with std::make_shared.
//
// To begin receiving, pass it a socket via initialize().
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
    virtual void receive_disconnect(const uvw::ErrorEvent &error) = 0;

    friend class NetworkClient;
};

class NetworkClient : public std::enable_shared_from_this<NetworkClient>
{
  public:
    NetworkClient(NetworkHandler *handler);
    ~NetworkClient();

    inline void initialize(SocketPtr socket)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        initialize(socket, lock);
    }
    inline void initialize(SocketPtr socket,
                           const uvw::Addr &remote,
                           const uvw::Addr &local)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        initialize(socket, remote, local, lock);
    }

    void set_write_timeout(unsigned int timeout);

    // send_datagram immediately sends the datagram over TCP (blocking).
    void send_datagram(DatagramHandle dg);
    // disconnect closes the TCP connection
    inline void disconnect()
    {
        uvw::ErrorEvent error(0);
        disconnect(error);
    }
    inline void disconnect(const uvw::ErrorEvent& error)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        disconnect(error, lock);
    }
    // is_connected returns true if the TCP connection is active, or false otherwise
    inline bool is_connected()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return is_connected(lock);
    }

    inline uvw::Addr get_remote()
    {
        return m_remote;
    }
    inline uvw::Addr get_local()
    {
        return m_local;
    }

  private:
    // Locked versions of public functions:
    void initialize(SocketPtr socket,
                    std::unique_lock<std::mutex> &lock);
    void initialize(SocketPtr socket,
                    const uvw::Addr &remote,
                    const uvw::Addr &local,
                    std::unique_lock<std::mutex> &lock);
    void disconnect(const uvw::ErrorEvent& error, std::unique_lock<std::mutex> &lock);
    bool is_connected(std::unique_lock<std::mutex> &lock);

    /* Asynchronous call loop */
    // async_send is called by send_datagram or send_finished when the socket is available
    //     for writing to send the next datagram in the queue.
    void async_send(DatagramHandle dg, std::unique_lock<std::mutex> &lock);
    // send_finished is called when an async_send has completed
    void send_finished();
    // send_expired is called when an async_send has expired
    void send_expired();

    // determine_endpoints is called by initialize() after m_socket is set to a
    //     provided socket.
    void determine_endpoints(uvw::Addr &remote,
                             uvw::Addr &local,
                             std::unique_lock<std::mutex> &lock);

    // async_receive is called by initialize() to begin receiving data, then by receive_size
    //     or receive_data to wait for the next set of data.
    void async_receive(std::unique_lock<std::mutex> &lock);

    // receive_size is called by async_receive when receiving the datagram size
    void receive_size(std::vector<uint8_t>& data);
    // receive_data is called by async_receive when receiving the datagram data
    void receive_data(std::vector<uint8_t>& data);

    typedef void (NetworkClient::*receive_handler_t)(std::vector<uint8_t>&);

    void socket_read(size_t length, receive_handler_t callback, std::unique_lock<std::mutex> &lock);
    void socket_write(uint8_t* buf, size_t length, std::unique_lock<std::mutex> &lock);

    void handle_disconnect(const uvw::ErrorEvent& error, std::unique_lock<std::mutex> &lock);

    bool m_is_sending = false;

    bool m_is_data = false;

    NetworkHandler *m_handler;

    SocketPtr m_socket;
    uvw::Addr m_remote;
    uvw::Addr m_local;
    std::shared_ptr<uvw::TimerHandle> m_async_timer;
    dgsize_t m_data_size = 0;

    unsigned int m_write_timeout = 0;

    std::mutex m_mutex;

    bool m_disconnect_handled = false;
    bool m_local_disconnect = false;
    int m_disconnect_error;
};
