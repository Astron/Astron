#include "NetworkClient.h"
#include <stdexcept>
#include "core/global.h"
#include "config/ConfigVariable.h"
using namespace std;

NetworkClient::NetworkClient(NetworkHandler *handler) : m_handler(handler),
   m_socket(nullptr), m_disconnect_error(0)
{
    auto loop = uvw::Loop::getDefault();
    m_async_timer = loop->resource<uvw::TimerHandle>();
}

NetworkClient::~NetworkClient()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(!is_connected(lock));

    m_socket->clear();
    m_socket = nullptr;
}

void NetworkClient::initialize(SocketPtr socket, std::unique_lock<std::mutex> &lock)
{
    if(m_socket) {
        throw std::logic_error("Trying to set a socket of a network client whose socket was already set.");
    }
    m_socket = socket;

    m_socket->set_error_callback([ptr = shared_from_this()](const uvw::ErrorEvent& error) {
        std::unique_lock<std::mutex> lock(ptr->m_mutex);
        ptr->handle_disconnect(error, lock);
    });

    bool endpoints_set = (m_remote.port && m_local.port);
    if (!endpoints_set) {
        determine_endpoints(m_remote, m_local, lock);
    }

    async_receive(lock);
}

void NetworkClient::initialize(SocketPtr socket,
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

void NetworkClient::determine_endpoints(uvw::Addr &remote, uvw::Addr &local,
                                        std::unique_lock<std::mutex> &)
{
    m_socket->determine_endpoints(remote, local);
}

void NetworkClient::set_write_timeout(unsigned int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_write_timeout = timeout;
}

void NetworkClient::send_datagram(DatagramHandle dg)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    async_send(dg, lock);
}

bool NetworkClient::is_connected(std::unique_lock<std::mutex> &)
{
    return !m_disconnect_handled && m_socket && m_socket->active();
}

void NetworkClient::async_receive(std::unique_lock<std::mutex> &lock)
{
    size_t bytes = m_is_data ? (size_t)m_data_size : sizeof(dgsize_t);
    if (m_socket->has_data_available(bytes))
    {
        // If the callback will be called right now,
        // we must release the lock to prevent a deadlock.
        lock.unlock();
    }

    m_socket->read(bytes, [ptr = shared_from_this()](std::vector<uint8_t>& data) {
        if (ptr->m_disconnect_handled) {
            // We got disconnected, ignore this event.
            return;
        }

        if (ptr->m_is_data) {
            ptr->receive_data(data);
        } else {
            ptr->receive_size(data);
        }
    });
}

void NetworkClient::disconnect(const uvw::ErrorEvent& error, std::unique_lock<std::mutex> & lock)
{
    if(m_local_disconnect || m_disconnect_handled) {
        // We've already set the error code and closed the socket; wait.
        return;
    }

    m_local_disconnect = true;
    m_disconnect_error = error.code();

    m_async_timer->stop();
    m_async_timer->clear();
    m_async_timer->close();

    handle_disconnect(error, lock);
}

void NetworkClient::handle_disconnect(const uvw::ErrorEvent& error,
                                      std::unique_lock<std::mutex> &lock)
{
    if(m_disconnect_handled) {
        return;
    }
    m_disconnect_handled = true;

    m_socket->clear();

    // Do NOT hold the lock when calling this. Our handler may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    lock.unlock();
    if(m_local_disconnect) {
        m_handler->receive_disconnect(uvw::ErrorEvent{m_disconnect_error});
    } else {
        m_handler->receive_disconnect(error);
    }
}

void NetworkClient::receive_size(std::vector<uint8_t>& data)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // required to disable strict-aliasing optimizations, which can break the code
    dgsize_t* size_buf = (dgsize_t*)&data[0];
    dgsize_t* new_size_p = (dgsize_t*)size_buf;
    m_data_size = swap_le(*new_size_p);
    if (!m_data_size)
    {
        handle_disconnect(uvw::ErrorEvent{(int)UV_EINVAL}, lock);
        return;
    }

    m_is_data = true;
    async_receive(lock);
}

void NetworkClient::receive_data(std::vector<uint8_t>& data)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    DatagramPtr dg;

    dg = Datagram::create(data); // Datagram makes a copy

    // Do NOT hold the lock when calling this. Our handler may acquire a
    // lock of its own, and the network lock should always be the lowest in the
    // lock hierarchy.
    lock.unlock();
    m_handler->receive_datagram(dg);
    lock.lock();

    m_is_data = false;

    async_receive(lock);
}

void NetworkClient::async_send(DatagramHandle dg, std::unique_lock<std::mutex> &lock)
{
    if (m_disconnect_handled) {
        // We got disconnected, ignore it.
        return;
    }

    size_t buffer_size = sizeof(dgsize_t) + dg->size();
    dgsize_t len = swap_le(dg->size());
    auto send_buf = new uint8_t[buffer_size];
    memcpy(send_buf, (uint8_t*)&len, sizeof(dgsize_t));
    memcpy(send_buf + sizeof(dgsize_t), dg->get_data(), dg->size());

    socket_write(send_buf, buffer_size, lock);
}

void NetworkClient::send_finished()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    // Cancel the outstanding timeout
    m_async_timer->stop();
}

void NetworkClient::send_expired()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    handle_disconnect(uvw::ErrorEvent{(int)UV_ETIMEDOUT}, lock);
}

void NetworkClient::socket_write(uint8_t* buf, size_t length, std::unique_lock<std::mutex> &)
{
    // Start async timeout, a value of 0 indicates the writes shouldn't timeout (used in debugging)
    if(m_write_timeout > 0) {
        m_async_timer->stop();
        m_async_timer->start(std::chrono::milliseconds{m_write_timeout}, std::chrono::milliseconds{0});
    }

    // Start async write
    m_socket->write(buf, length, [ptr = shared_from_this()]() {
        ptr->send_finished();
    });
}
