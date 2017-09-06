#include "SocketWrapper.h"

SocketWrapper::SocketWrapper(std::shared_ptr<uvw::TcpHandle> socket) : m_initialized(false),
  m_socket(socket), m_callback(nullptr), m_read_size(0)
{
}

SocketWrapper::~SocketWrapper()
{
    clear();
    m_callback = nullptr;
    m_data_buf.clear();
}

void SocketWrapper::initialize()
{
    if (m_initialized)
        return;

    m_initialized = true;

    m_socket->clear();
    m_socket->keepAlive(true);
    m_socket->blocking(false);

    m_socket->on<uvw::ErrorEvent>([ptr = shared_from_this()](const uvw::ErrorEvent& error, const auto&) {
        ptr->m_error_callback(error);
    });

    m_socket->on<uvw::EndEvent>([ptr = shared_from_this()](const auto&, const auto&) {
        if (ptr->m_error_callback)
        {
            ptr->m_error_callback(uvw::ErrorEvent{(int)UV_EOF});
        }
    });

    m_socket->on<uvw::DataEvent>([ptr = shared_from_this()](const uvw::DataEvent& event, const auto&) {
        ptr->handle_data(event);
    });

    m_socket->read();
}

void SocketWrapper::set_error_callback(ErrorCallback error_callback)
{
    m_error_callback = error_callback;
}

void SocketWrapper::clear_error_callback()
{
    m_error_callback = nullptr;
}

void SocketWrapper::determine_endpoints(uvw::Addr& remote, uvw::Addr& local)
{
    remote = m_socket->peer();
    local = m_socket->sock();
}

void SocketWrapper::write(uint8_t* buf, size_t bytes, WriteCallback callback)
{
    if (!m_socket)
    {
        return;
    }

    m_socket->write(std::move((char*)buf), bytes);
    m_socket->once<uvw::WriteEvent>([callback](const auto&, const auto&) {
        callback();
    });
}

void SocketWrapper::read(size_t bytes, SocketReadCallback callback)
{
    if (m_read_size)
    {
        throw std::logic_error("Still processing previous read request.");
    }

    if (!m_socket)
    {
        return;
    }

    // Can we serve it right now?
    if (has_data_available(bytes))
    {
        // Extract <bytes> bytes
        std::vector<uint8_t> data(m_data_buf.begin(), m_data_buf.begin() + bytes);
        m_data_buf.erase(m_data_buf.begin(), m_data_buf.begin() + bytes);

        callback(data);
        return;
    }

    m_read_size = bytes;
    m_callback = callback;
}

void SocketWrapper::handle_error(const uvw::ErrorEvent& error)
{
    if (m_error_callback)
    {
        m_error_callback(error);
    }
}

void SocketWrapper::handle_data(const uvw::DataEvent &event)
{
    // Store received data in m_data_buf
    m_data_buf.reserve(m_data_buf.size() + event.length);
    m_data_buf.insert(m_data_buf.end(), (uint8_t*)event.data.get(), (uint8_t*)(event.data.get() + event.length));

    // Can we call read callback?
    if (m_read_size && has_data_available(m_read_size))
    {
        // Extract <m_read_size> bytes
        std::vector<uint8_t> data(m_data_buf.begin(), m_data_buf.begin() + m_read_size);
        m_data_buf.erase(m_data_buf.begin(), m_data_buf.begin() + m_read_size);

        m_read_size = 0;

        m_callback(data);
    }
}

void SocketWrapper::clear()
{
    if (m_socket)
    {
        m_socket->clear();
        m_socket->close();
        m_socket = nullptr;
    }

    clear_error_callback();
}
