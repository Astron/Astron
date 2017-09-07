#include "NetworkClient.h"
#include <stdexcept>
#include "core/global.h"
#include "config/ConfigVariable.h"

NetworkClient::NetworkClient(NetworkHandler *handler) : m_handler(handler), m_socket(nullptr), m_send_queue(), 
                                                        m_disconnect_error(0)
{
}

NetworkClient::~NetworkClient()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(!is_connected(lock));

    m_socket = nullptr;
    delete [] m_send_buf;
}

void NetworkClient::initialize(const std::shared_ptr<uvw::TcpHandle>& socket, std::unique_lock<std::mutex> &lock)
{
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }
    m_socket = socket;

    m_socket->noDelay(true);
    m_socket->keepAlive(true, uvw::TcpHandle::Time{60});

    bool endpoints_set = (m_remote.port && m_local.port);
    if(endpoints_set || determine_endpoints(m_remote, m_local)) {
        async_receive();
    }
}

void NetworkClient::initialize(const std::shared_ptr<uvw::TcpHandle>& socket,
                               const uvw::Addr &remote,
                               const uvw::Addr &local,
                               std::unique_lock<std::mutex> &lock)
{
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }
    m_remote = remote;
    m_local = local;

    initialize(socket, lock);
}

bool NetworkClient::determine_endpoints(uvw::Addr &remote, uvw::Addr &local)
{
    remote = m_socket->peer();
    local = m_socket->sock();
    return true;
}

void NetworkClient::set_write_timeout(unsigned int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_write_timeout = timeout;
}

void NetworkClient::set_write_buffer(uint64_t max_bytes)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_max_queue_size = max_bytes;
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_is_sending) {
        m_send_queue.push(dg);
        m_total_queue_size += dg->size();
        if(m_total_queue_size > m_max_queue_size && m_max_queue_size != 0) {
            disconnect(UV_ENOBUFS, lock);
        }
    } else {
        m_is_sending = true;
        async_send(dg, lock);
    }
}

bool NetworkClient::is_connected(std::unique_lock<std::mutex> &)
{
    return m_socket && m_socket->readable() && m_socket->writable();
}

void NetworkClient::defragment_input()
{
    while(m_data_buf.size() > sizeof(dgsize_t)) {
        // Enough data to know the expected length of the datagram.
        dgsize_t data_size = *reinterpret_cast<dgsize_t*>(&m_data_buf[0]);
        if(m_data_buf.size() >= data_size + sizeof(dgsize_t)) {
            dgsize_t overread_size = (m_data_buf.size() - data_size - sizeof(dgsize_t));
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(&m_data_buf[0] + sizeof(dgsize_t)), data_size);
            if(overread_size > 0) {
                // Splice leftover data to new m_data_buf based on expected datagram length.
                m_data_buf = std::vector<unsigned char>(reinterpret_cast<char*>(&m_data_buf[0] + sizeof(dgsize_t) + data_size),
                                                        reinterpret_cast<char*>(&m_data_buf[0] + sizeof(dgsize_t) + data_size + overread_size));
            } else {
                // No overread, buffer is empty.
                m_data_buf = std::vector<unsigned char>();
            }
            m_handler->receive_datagram(dg);
        }
        else {
            break;
        }
    }
}

void NetworkClient::process_datagram(const std::unique_ptr<char[]>& data, size_t size)
{
    if(m_data_buf.size() == 0 && size >= 2) {
        // Fast-path mode: Check if we have just enough data from the stream for a single datagram.
        dgsize_t datagram_size = *reinterpret_cast<dgsize_t*>(data.get());
        if(datagram_size == size - sizeof(dgsize_t)) {
            // Yep. Dispatch to receive_datagram and early-out.
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(data.get() + sizeof(dgsize_t)), datagram_size);
            m_handler->receive_datagram(dg);
            return;
        }
    }

    m_data_buf.insert(m_data_buf.end(), data.get(), data.get() + size);
    defragment_input();
}

void NetworkClient::async_receive()
{
    // The lambda below runs within the context of the event loop thread.
    // We do not have to worry about locking as long as nothing touches m_data_buf besides process_datagram.
    m_socket->on<uvw::DataEvent>([this](const uvw::DataEvent &event, uvw::TcpHandle &) {
        this->process_datagram(event.data, event.length);
    });

    m_socket->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& event, uvw::TcpHandle &) {
        this->handle_disconnect((uv_errno_t)event.code());
    });

    m_socket->on<uvw::EndEvent>([this](const uvw::EndEvent& event, uvw::TcpHandle &) {
        this->handle_disconnect(UV_EOF);
    });

    m_socket->read();
}

void NetworkClient::disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &)
{
    if(m_local_disconnect || m_disconnect_handled) {
        // We've already set the error code and closed the socket; wait.
        return;
    }

    m_local_disconnect = true;
    m_disconnect_error = ec;

    m_socket->close();
}

void NetworkClient::handle_disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock)
{
    if(m_disconnect_handled) {
        return;
    }

    m_disconnect_handled = true;

    if(is_connected(lock)) {
        m_socket->close();
    }

    // Do NOT hold the lock when calling this. Our handler may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    lock.unlock();
    if(m_local_disconnect) {
        m_handler->receive_disconnect(uvw::ErrorEvent((int)m_disconnect_error));
    } else {
        m_handler->receive_disconnect(uvw::ErrorEvent((int)ec));
    }
}

void NetworkClient::async_send(DatagramHandle dg, std::unique_lock<std::mutex> &lock)
{
    /*
    size_t buffer_size = sizeof(dgsize_t) + dg->size();
    dgsize_t len = swap_le(dg->size());
    m_send_buf = new uint8_t[buffer_size];
    memcpy(m_send_buf, (uint8_t*)&len, sizeof(dgsize_t));
    memcpy(m_send_buf + sizeof(dgsize_t), dg->get_data(), dg->size());

    try {
        socket_write(m_send_buf, buffer_size, lock);
    } catch(const boost::system::system_error& err) {
        // An exception happening when trying to initiate a send is a clear
        // indicator that something happened to the connection, therefore:
        disconnect(err.code(), lock);
    }
    */
}

void NetworkClient::send_finished()
{
    /*
    std::unique_lock<std::mutex> lock(m_mutex);

    // Cancel the outstanding timeout
    m_async_timer.cancel();

    // Discard the buffer we just used:
    delete [] m_send_buf;
    m_send_buf = nullptr;

    // Check if the write had errors
    if(ec.value() != 0) {
        m_is_sending = false;
        disconnect(ec, lock);
        return;
    }

    // Check if we have more items in the queue
    if(m_send_queue.size() > 0) {
        // Send the next item in the queue
        DatagramHandle dg = m_send_queue.front();
        m_total_queue_size -= dg->size();
        m_send_queue.pop();
        async_send(dg, lock);
        return;
    }

    // Nothing left in the queue to send, lets open up for another write
    m_is_sending = false;
    */
}

void NetworkClient::send_expired()
{
    /*
    std::unique_lock<std::mutex> lock(m_mutex);

    // operation_aborted is received if the the timer is cancelled,
    //     ie. if the send completed before it expires, so don't do anything
    if(ec != boost::asio::error::operation_aborted) {
        boost::system::error_code etimeout(boost::system::errc::errc_t::timed_out,
                                           boost::system::system_category());
        disconnect(etimeout, lock);
    }
    */
}

void NetworkClient::socket_write(const uint8_t* buf, size_t length, std::unique_lock<std::mutex> &)
{
    /*
    // Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
    if(m_write_timeout > 0) {
        m_async_timer.expires_from_now(boost::posix_time::milliseconds(m_write_timeout));
        m_async_timer.async_wait(boost::bind(&NetworkClient::send_expired, shared_from_this(),
                                             boost::asio::placeholders::error));
    }

    async_write(*m_socket, boost::asio::buffer(buf, length),
                boost::bind(&NetworkClient::send_finished, shared_from_this(),
                            boost::asio::placeholders::error));
    */
}
