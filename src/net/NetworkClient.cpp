#include "NetworkClient.h"
#include <stdexcept>
#include "core/global.h"
#include "config/ConfigVariable.h"

NetworkClient::NetworkClient(NetworkHandler *handler) : m_handler(handler), m_socket(nullptr),
                                                        m_async_timer(), m_send_queue(),
                                                        m_disconnect_error(UV_EOF)
{
}

NetworkClient::~NetworkClient()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_socket->close();
    m_socket = nullptr;
    delete [] m_send_buf;
}

void NetworkClient::initialize(const std::shared_ptr<uvw::TcpHandle>& socket, std::unique_lock<std::mutex> &)
{
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }

    m_socket = socket;

    m_socket->noDelay(true);
    m_socket->keepAlive(true, uvw::TcpHandle::Time{60});

    m_async_timer = g_loop->resource<uvw::TimerHandle>();

    determine_endpoints(m_remote, m_local);

    start_receive();
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

    if(std::this_thread::get_id() != g_main_thread_id) {
        std::shared_ptr<uvw::AsyncHandle> handle = g_loop->resource<uvw::AsyncHandle>();

        handle->on<uvw::AsyncEvent>([this, dg](const uvw::AsyncEvent&, uvw::AsyncHandle& handle) {
            this->send_datagram(dg);
            handle.close();
        });

        handle->send();
        return;
    }

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
    return m_socket && m_socket->active();
}

void NetworkClient::defragment_input()
{
    while(m_data_buf.size() > sizeof(dgsize_t)) {
        // Enough data to know the expected length of the datagram.
        dgsize_t data_size = *reinterpret_cast<dgsize_t*>(&m_data_buf.front());
        if(m_data_buf.size() >= data_size + sizeof(dgsize_t)) {
            dgsize_t overread_size = (m_data_buf.size() - data_size - sizeof(dgsize_t));
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(&m_data_buf.front() + sizeof(dgsize_t)), data_size);
            if(overread_size > 0) {
                // Splice leftover data to new m_data_buf based on expected datagram length.
                m_data_buf = std::vector<unsigned char>(reinterpret_cast<char*>(&m_data_buf.front() + sizeof(dgsize_t) + data_size),
                                                        reinterpret_cast<char*>(&m_data_buf.front() + sizeof(dgsize_t) + data_size + overread_size));
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
    /*
    if(m_data_buf.size() == 0 && size >= sizeof(dgsize_t)) {
        // Fast-path mode: Check if we have just enough data from the stream for a single datagram.
        // Should occur in most cases, as we're expecting <= the average TCP MSS for most datagrams.
        dgsize_t datagram_size = *reinterpret_cast<dgsize_t*>(data.get());
        if(datagram_size == size - sizeof(dgsize_t)) {
            // Yep. Dispatch to receive_datagram and early-out.
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(data.get() + sizeof(dgsize_t)), datagram_size);
            m_handler->receive_datagram(dg);
            return;
        }
    }
    */

    m_data_buf.insert(m_data_buf.end(), data.get(), data.get() + size);
    defragment_input();
}

void NetworkClient::start_receive()
{
    // Sets up all the handlers needed for the NetworkClient instance and starts receiving data from the stream.

    // The lambda below runs within the context of the event loop thread.
    // We do not have to worry about locking as long as nothing touches m_data_buf besides process_datagram.
    m_socket->on<uvw::DataEvent>([this](const uvw::DataEvent &event, uvw::TcpHandle &) {
        this->process_datagram(event.data, event.length);
    });

    m_socket->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& event, uvw::TcpHandle &) {
        this->handle_disconnect((uv_errno_t)event.code());
    });

    m_socket->on<uvw::EndEvent>([this](const uvw::EndEvent&, uvw::TcpHandle &) {
        this->handle_disconnect(UV_EOF);
    });

    m_socket->on<uvw::CloseEvent>([this](const uvw::CloseEvent&, uvw::TcpHandle &) {
        this->handle_disconnect(UV_EOF);
    });

    m_socket->on<uvw::WriteEvent>([this](const uvw::WriteEvent&, uvw::TcpHandle &) {
        this->send_finished();
    });

    m_async_timer->on<uvw::TimerEvent>([this](const uvw::TimerEvent&, uvw::TimerHandle &) {
        this->send_expired();
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
    m_async_timer->stop();
}

void NetworkClient::handle_disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock)
{
    if(m_disconnect_handled) {
        return;
    }

    m_disconnect_handled = true;

    m_socket->close();
    m_async_timer->stop();

    // Do NOT hold the lock when calling this. Our handler may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    lock.unlock();
    if(m_local_disconnect) {
        m_handler->receive_disconnect(uvw::ErrorEvent{(int)m_disconnect_error});
    } else {
        m_handler->receive_disconnect(uvw::ErrorEvent{(int)ec});
    }
}

void NetworkClient::async_send(DatagramHandle dg, std::unique_lock<std::mutex> &lock)
{
    size_t buffer_size = sizeof(dgsize_t) + dg->size();
    dgsize_t len = swap_le(dg->size());
    m_send_buf = new char[buffer_size];
    memcpy(m_send_buf, (char*)&len, sizeof(dgsize_t));
    memcpy(m_send_buf + sizeof(dgsize_t), dg->get_data(), dg->size());

    socket_write(m_send_buf, buffer_size, lock);
}

void NetworkClient::send_finished()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // Cancel the outstanding timeout
    m_async_timer->stop();

    // Discard the buffer we just used:
    delete [] m_send_buf;
    m_send_buf = nullptr;

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
}

void NetworkClient::send_expired()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    disconnect(UV_ETIMEDOUT, lock);
}

void NetworkClient::socket_write(char* buf, size_t length, std::unique_lock<std::mutex> &)
{
    // Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
    if(m_write_timeout > 0) {
        m_async_timer->stop();
        m_async_timer->start(uvw::TimerHandle::Time{m_write_timeout}, uvw::TimerHandle::Time{0});
    }

    m_socket->write(buf, length);
}
