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

    assert(!is_connected(lock));

    shutdown(lock);

    if(m_send_buf != nullptr) {
        delete [] m_send_buf;
        m_send_buf = nullptr;
    }
}

void NetworkClient::shutdown(std::unique_lock<std::mutex> &lock)
{
    if(m_socket == nullptr || m_async_timer == nullptr) {
        return;
    }

    auto socket = m_socket;
    auto async_timer = m_async_timer;

    lock.unlock();
    TaskQueue::singleton.enqueue_task([=]() {
        socket->close();
        async_timer->stop();
        async_timer->close();
    });
    lock.lock();

    m_socket = nullptr;
    m_async_timer = nullptr;
    m_haproxy_handler = nullptr;
}

void NetworkClient::initialize(const std::shared_ptr<uvw::TcpHandle>& socket,
                               const uvw::Addr &remote,
                               const uvw::Addr &local,
                               const bool haproxy_mode,
                               std::unique_lock<std::mutex> &lock)
{
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }

    // This function should ONLY run in the main thread. libuv is not thread-safe.
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_socket = socket;

    m_socket->noDelay(true);
    m_socket->keepAlive(true, uvw::TcpHandle::Time{60});

    m_async_timer = g_loop->resource<uvw::TimerHandle>();

    m_remote = remote;
    m_local = local;

    m_haproxy_mode = haproxy_mode;

    // Slight deviation between behaviour in HAProxy mode and "regular" CA mode:
    // In HAProxy mode, we want to wait for HAProxyHandler to execute before running the NetworkHandler's initialize().
    if(m_haproxy_mode) {
        m_haproxy_handler = std::make_unique<HAProxyHandler>();
    }
    else {
        lock.unlock();
        m_handler->initialize();
        lock.lock();
    }

    // NOT protected by a lock, make sure it runs in main!
    start_receive();
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // If we aren't connected, stop here.
    if(!is_connected(lock)) {
        return;
    }

    // Put the packet in our outgoing send queue
    m_send_queue.push_back(dg);

    // Check our quota, disconnect if it's too much
    m_total_queue_size += dg->size();
    if(m_total_queue_size > m_max_queue_size && m_max_queue_size != 0) {
        disconnect(UV_ENOBUFS, lock);
        return;
    }

    // Poke the main thread to flush its buffer (it's fine if this is called
    // twice, it checks if it's already sending)
    if(g_main_thread_id != std::this_thread::get_id()) {
        lock.unlock();
        TaskQueue::singleton.enqueue_task([self = shared_from_this()] () {
            std::unique_lock<std::mutex> lock(self->m_mutex);
            self->flush_send_queue(lock);
        });
    } else {
        flush_send_queue(lock);
    }
}

void NetworkClient::defragment_input(std::unique_lock<std::mutex>& lock)
{
    while(m_data_buf.size() > sizeof(dgsize_t)) {
        // Enough data to know the expected length of the datagram.
        dgsize_t data_size = *reinterpret_cast<dgsize_t*>(&m_data_buf[0]);
        if(m_data_buf.size() >= data_size + sizeof(dgsize_t)) {
            auto overread_size = (m_data_buf.size() - data_size - sizeof(dgsize_t));
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(&m_data_buf[sizeof(dgsize_t)]), data_size);
            if(0 < overread_size) {
                // Splice leftover data to new m_data_buf based on expected datagram length.
                m_data_buf = std::vector<unsigned char>(reinterpret_cast<char*>(&m_data_buf[sizeof(dgsize_t) + data_size]),
                                                        reinterpret_cast<char*>(&m_data_buf[sizeof(dgsize_t) + data_size + overread_size]));
            } else {
                // No overread, buffer is empty.
                m_data_buf = std::vector<unsigned char>();
            }

            lock.unlock();
            m_handler->receive_datagram(dg);
            lock.lock();
        }
        else {
            return;
        }
    }
}

void NetworkClient::process_datagram(const std::unique_ptr<char[]>& data, size_t size)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // This function should ONLY run in the main thread. It's a libuv event.
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_data_buf.size() == 0 && size >= sizeof(dgsize_t)) {
        // Fast-path mode: Check if we have just enough data from the stream for a single datagram.
        // Should occur in most cases, as we're expecting <= the average TCP MSS for most datagrams.
        dgsize_t datagram_size = *reinterpret_cast<dgsize_t*>(data.get());
        if(datagram_size == size - sizeof(dgsize_t)) {
            // Yep. Dispatch to receive_datagram and early-out.
            DatagramPtr dg = Datagram::create(reinterpret_cast<const uint8_t*>(data.get() + sizeof(dgsize_t)), datagram_size);
            lock.unlock();
            m_handler->receive_datagram(dg);
            return;
        }
    }

    m_data_buf.insert(m_data_buf.end(), data.get(), data.get() + size);
    defragment_input(lock);
}

void NetworkClient::start_receive()
{
    // Sets up all the handlers needed for the NetworkClient instance and starts receiving data from the stream.
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_socket->on<uvw::DataEvent>([self = shared_from_this()](const uvw::DataEvent &event, uvw::TcpHandle &) {
        if(self->m_haproxy_handler != nullptr) {
            size_t bytes_consumed = self->m_haproxy_handler->consume(reinterpret_cast<const uint8_t*>(event.data.get()), event.length);
            if(bytes_consumed < event.length || bytes_consumed == 0) {
                if(self->m_haproxy_handler->has_error()) {
                    // An error occured while processing the HAProxy headers.
                    // Disconnect the client with the error code passed down as the reason, and destroy the HAProxyHandler instance without doing anything else.
                    self->disconnect(self->m_haproxy_handler->get_error());
                    self->m_haproxy_handler = nullptr;
                    return;
                }

                self->m_is_local = self->m_haproxy_handler->is_local();
                if(!self->m_is_local) {
                    self->m_local = self->m_haproxy_handler->get_local();
                    self->m_remote = self->m_haproxy_handler->get_remote();
                    self->m_tlv_buf = self->m_haproxy_handler->get_tlvs();
                }

                self->m_haproxy_handler = nullptr;
                self->m_handler->initialize();

                ssize_t bytes_left = bytes_consumed > 0 ? event.length - bytes_consumed : 0;
                if(0 < bytes_left) {
                    // Feed any left-over bytes (if any) back to process_datagram.
                    std::unique_ptr<char[]> overread_bytes = std::make_unique<char[]>(bytes_left);
                    memcpy(overread_bytes.get(), event.data.get() + bytes_consumed, bytes_left);
                    self->process_datagram(overread_bytes, bytes_left);
                } 
            }
        } else {
            self->process_datagram(event.data, event.length);
        }
    });

    m_socket->on<uvw::ErrorEvent>([self = shared_from_this()](const uvw::ErrorEvent& event, uvw::TcpHandle &) {
        self->handle_disconnect((uv_errno_t)event.code());
    });

    m_socket->on<uvw::EndEvent>([self = shared_from_this()](const uvw::EndEvent&, uvw::TcpHandle &) {
        self->handle_disconnect(UV_EOF);
    });

    m_socket->on<uvw::CloseEvent>([self = shared_from_this()](const uvw::CloseEvent&, uvw::TcpHandle &) {
        self->handle_disconnect(UV_EOF);
    });

    m_socket->on<uvw::WriteEvent>([self = shared_from_this()](const uvw::WriteEvent&, uvw::TcpHandle &) {
        self->send_finished();
    });

    m_async_timer->on<uvw::TimerEvent>([self = shared_from_this()](const uvw::TimerEvent&, uvw::TimerHandle &) {
        self->send_expired();
    });

    m_socket->read();
}

void NetworkClient::disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock)
{
    if(m_local_disconnect || m_disconnect_handled) {
        // We've already set the error code and closed the socket; wait.
        return;
    }

    m_local_disconnect = true;
    m_disconnect_error = ec;

    if(!m_is_sending && m_total_queue_size == 0) {
        // Nothing left to send out, shutdown the socket immediately.
        shutdown(lock);
    } else {
        // Let flush_send_queue execute first:
        // The send_finished callback is responsible for closing the socket at the end of the flush.
        if(g_main_thread_id != std::this_thread::get_id()) {
            lock.unlock();
            TaskQueue::singleton.enqueue_task([self = shared_from_this()] () {
                std::unique_lock<std::mutex> lock(self->m_mutex);
                self->flush_send_queue(lock);
            });
        } else {
            flush_send_queue(lock);
        }
    }
}

void NetworkClient::handle_disconnect(uv_errno_t ec, std::unique_lock<std::mutex> &lock)
{
    // This function should ONLY run in the main thread. It's a libuv event.
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_disconnect_handled) {
        return;
    }

    m_disconnect_handled = true;

    shutdown(lock);

    // Do NOT hold the lock when calling this. Our handler may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    lock.unlock();
    if(m_local_disconnect) {
        m_handler->receive_disconnect(uvw::ErrorEvent{(int)m_disconnect_error});
    } else {
        m_handler->receive_disconnect(uvw::ErrorEvent{(int)ec});
    }

    lock.lock();
}

void NetworkClient::flush_send_queue(std::unique_lock<std::mutex> &lock)
{
    // libuv is NOT thread-safe. This function must ONLY be called in the main
    // thread.
    assert(std::this_thread::get_id() == g_main_thread_id);

    // If we aren't connected, stop here
    if(!is_connected(lock)) {
        return;
    }

    // Are we sending already?
    if(m_is_sending) {
        return;
    }

    // Do we have anything to send?
    if(!m_send_queue.size()) {
        assert(m_total_queue_size == 0);
        return;
    }

    auto socket = m_socket;

    // Figure out how big of a send buffer we need
    size_t buffer_size = 0;
    for(auto dg : m_send_queue) {
        buffer_size += sizeof(dgsize_t) + dg->size();
    }
    assert(m_send_buf == nullptr);
    m_send_buf = new char[buffer_size];

    // Fill the send buffer with our data:
    char *send_ptr = &m_send_buf[0];
    for(auto dg : m_send_queue) {
        // Add the size tag:
        dgsize_t len = swap_le(dg->size());
        memcpy(send_ptr, (char*)&len, sizeof(dgsize_t));
        send_ptr += sizeof(dgsize_t);

        // Add the data:
        memcpy(send_ptr, dg->get_data(), dg->size());
        send_ptr += dg->size();

        // Discount it from our send queue:
        m_total_queue_size -= dg->size();
    }
    assert(send_ptr == &m_send_buf[buffer_size]);

    // Clean up our m_send_queue:
    assert(m_total_queue_size == 0);
    m_send_queue.clear();

    // Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
    if(m_write_timeout > 0) {
        m_async_timer->stop();
        m_async_timer->start(uvw::TimerHandle::Time{m_write_timeout}, uvw::TimerHandle::Time{0});
    }

    // Bombs away!
    m_is_sending = true;
    lock.unlock();
    socket->write(m_send_buf, buffer_size);
    lock.lock();
}

void NetworkClient::send_finished()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // This function should ONLY run in the main thread. It's a libuv event.
    assert(std::this_thread::get_id() == g_main_thread_id);

    // Mark ourselves as "not sending"
    assert(m_is_sending);
    m_is_sending = false;

    // Discard the buffer we just used:
    assert(m_send_buf != nullptr);
    delete [] m_send_buf;
    m_send_buf = nullptr;

    // If we aren't connected, stop here
    if(!is_connected(lock)) {
        return;
    }

    // Cancel the outstanding timeout:
    m_async_timer->stop();

    // If we've had a local disconnect and there are no pending buffers to send, stop here
    if(m_local_disconnect && m_total_queue_size == 0) {
        shutdown(lock);
        return;
    }

    // Flush more items out of the queue:
    flush_send_queue(lock);
}

void NetworkClient::send_expired()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // This function should ONLY run in the main thread. It's a libuv event.
    assert(std::this_thread::get_id() == g_main_thread_id);

    // We need to clean up after ourselves before invoking disconnect:
    // Otherwise we might inadvertedly end up hitting flush_send_queue, and we don't want to do that here.
    assert(m_is_sending);
    m_is_sending = false;

    assert(m_send_buf != nullptr);
    delete[] m_send_buf;
    m_send_buf = nullptr;

    m_total_queue_size = 0;
    m_send_queue.clear();

    disconnect(UV_ETIMEDOUT, lock);
}
