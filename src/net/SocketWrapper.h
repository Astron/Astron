#pragma once

#include <functional>

#include <uvw.hpp>

typedef std::function<void(std::vector<uint8_t>&)> SocketReadCallback;
typedef std::function<void()> WriteCallback;
typedef std::function<void(const uvw::ErrorEvent&)> ErrorCallback;

class SocketWrapper : public std::enable_shared_from_this<SocketWrapper>
{
  public:
    SocketWrapper(std::shared_ptr<uvw::TcpHandle> socket);
    ~SocketWrapper();

    void initialize();
    void set_error_callback(ErrorCallback error_callback);
    void clear_error_callback();
    void determine_endpoints(uvw::Addr& remote, uvw::Addr& local);

    inline bool active()
    {
        return m_socket->active();
    }

    inline bool has_data_available(size_t bytes)
    {
        return m_data_buf.size() >= bytes;
    }

    void write(uint8_t* buf, size_t bytes, WriteCallback callback);
    void read(size_t bytes, SocketReadCallback callback);

    void clear();

  private:
    void handle_error(const uvw::ErrorEvent& error);
    void handle_data(const uvw::DataEvent &event);

    bool m_initialized;

    std::shared_ptr<uvw::TcpHandle> m_socket;
    std::vector<uint8_t> m_data_buf;

    SocketReadCallback m_callback;
    ErrorCallback m_error_callback;
    size_t m_read_size;
};

typedef std::shared_ptr<SocketWrapper> SocketPtr;
