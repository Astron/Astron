#include "HAProxyHandler.h"
#include <boost/asio.hpp> // For boost::asio::ip;
#include <cstring>

using namespace boost::asio::ip;

static const uint8_t haproxy_signature_v1[] = {
    'P', 'R', 'O', 'X', 'Y', ' ', 'T', 'C', 'P'
};
static const uint8_t haproxy_signature_v2[] = {
    0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A
};

void HAProxyHandler::async_process(SocketPtr socket, ProxyCallback callback)
{
    HAProxyHandler *handler = new HAProxyHandler(socket, callback);
    handler->begin();
}

HAProxyHandler::HAProxyHandler(SocketPtr socket, ProxyCallback callback) :
    m_socket(socket), m_callback(callback)
{
    m_socket->determine_endpoints(m_remote, m_local);
}

HAProxyHandler::~HAProxyHandler()
{
    if(m_body_buf != nullptr) {
        delete [] m_body_buf;
    }
}

void HAProxyHandler::begin()
{
    m_socket->read(HAPROXY_HEADER_MIN, [this](std::vector<uint8_t>& data) {
        std::copy(data.begin(), data.end(), this->m_header_buf);
        this->handle_header();
    });
}

void HAProxyHandler::handle_header()
{
    if(!memcmp(m_header_buf, haproxy_signature_v2, sizeof(haproxy_signature_v2))) {
        // Yay, it's a binary proxy header! These are easy to process:
        m_body_len = (m_header_buf[14] << 8) | m_header_buf[15];
        m_body_buf = new uint8_t[m_body_len];
        m_socket->read(m_body_len, [this](std::vector<uint8_t>& data) {
            std::copy(data.begin(), data.end(), this->m_body_buf);
            this->handle_v2();
        });
    } else if(!memcmp(m_header_buf, haproxy_signature_v1, sizeof(haproxy_signature_v1))) {
        // This is the ASCII version (begins with "PROXY ", terminated by
        // "\r\n"). These are harder to process, since we don't know how much
        // proxy data there is in advance, and we can't risk reading even a
        // single byte past the end of the "\r\n" because that's going to be
        // application-layer data and we have no facility for putting it back
        // into the read buffer after we take it out.

        // This function deals with it.
        m_header_len += HAPROXY_HEADER_MIN;
        handle_v1();
    } else {
        // Couldn't recognize any proxy header.
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
    }
}

void HAProxyHandler::handle_v2()
{
    parse_v2();
}

void HAProxyHandler::parse_v2()
{
    uvw::ErrorEvent no_error(0);

    int version = (m_header_buf[12] >> 4) & 0xF;
    int command = (m_header_buf[12]) & 0xF;
    int family  = (m_header_buf[13] >> 4) & 0xF;
    int transp  = (m_header_buf[13]) & 0xF;

    if(version != 2) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    if(family != 0x1 && family != 0x2) {
        finish(uvw::ErrorEvent{(int)UV_EAFNOSUPPORT});
        return;
    }

    if(transp != 0x1) {
        finish(uvw::ErrorEvent{(int)UV_EPROTONOSUPPORT});
        return;
    }

    if(command == 0x0) {
        // LOCAL, ignore the body:
        finish(no_error);
        return;
    }

    if(command != 0x1) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    if((family == 0x1 && m_body_len < 12) ||
       (family == 0x2 && m_body_len < 36)) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    address remote_address;
    address local_address;
    int port_offset;

    if(family == 0x1) {
        address_v4::bytes_type address_bytes;

        memcpy(address_bytes.data(), &m_body_buf[0], 4);
        address_v4 remote_v4(address_bytes);

        memcpy(address_bytes.data(), &m_body_buf[4], 4);
        address_v4 local_v4(address_bytes);

        m_remote.ip = remote_v4.to_string();
        m_local.ip = local_v4.to_string();

        port_offset = 8;
    } else {
        address_v6::bytes_type address_bytes;

        memcpy(address_bytes.data(), &m_body_buf[0], 16);
        address_v6 remote_v6(address_bytes);

        memcpy(address_bytes.data(), &m_body_buf[16], 16);
        address_v6 local_v6(address_bytes);

        m_remote.ip = remote_v6.to_string();
        m_local.ip = local_v6.to_string();

        port_offset = 32;
    }

    m_remote.port = m_body_buf[port_offset] << 8 | m_body_buf[port_offset + 1];
    m_local.port = m_body_buf[port_offset + 2] << 8 | m_body_buf[port_offset + 3];

    finish(no_error);
}

void HAProxyHandler::handle_v1()
{
    // See if we got a '\n', indicating the end of the proxy header:
    if(memchr(m_header_buf, '\n', m_header_len) != nullptr) {
        parse_v1();
        return;
    }

    if(m_header_len >= HAPROXY_HEADER_MAX) {
        finish(uvw::ErrorEvent{(int)UV_EAI_OVERFLOW});
        return;
    }
    size_t remaining_size = HAPROXY_HEADER_MAX - m_header_len;

    // Decide how many more bytes we want:
    size_t read_size;
    if(memchr(m_header_buf, '\r', m_header_len) != nullptr) {
        // There's a '\r', so '\n' is imminent!
        read_size = 1;
    } else {
        // We can safely read at least 2 bytes. We also know that the finished
        // PROXY header will have 5 spaces, with at least one char before each
        // of the spaces. So we can read 2 + (5-number_of_spaces)*2 or
        // 12 - number_of_spaces*2
        read_size = 12;
        for(uint8_t *i = m_header_buf; i < &m_header_buf[m_header_len]; i++)
            if(*i == ' ')
                read_size -= 2;
    }

    m_socket->read(std::min(read_size, remaining_size), [this](std::vector<uint8_t>& data) {
        std::copy(data.begin(), data.end(), &this->m_header_buf[this->m_header_len]);
        this->m_header_len += data.size();
        this->handle_v1();
    });
}

void HAProxyHandler::parse_v1()
{
    if((m_header_buf[m_header_len - 2] != '\r') ||
       (m_header_buf[m_header_len - 1] != '\n')) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    m_header_buf[m_header_len] = '\0';

    /* const char *proxy = */ strtok((char*)m_header_buf, " ");
    const char *tcp = strtok(NULL, " ");
    const char *srcip = strtok(NULL, " ");
    const char *dstip = strtok(NULL, " ");
    const char *srcport = strtok(NULL, " ");
    const char *dstport = strtok(NULL, "\r");

    if(dstport == nullptr) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    if(strcmp(tcp, "TCP4") && strcmp(tcp, "TCP6")) {
        finish(uvw::ErrorEvent{(int)UV_EPROTO});
        return;
    }

    m_remote.ip = std::string(srcip);
    m_remote.port = atoi(srcport);

    m_local.ip = std::string(dstip);
    m_local.port = atoi(dstport);

    uvw::ErrorEvent no_error(0);
    finish(no_error);
}

void HAProxyHandler::finish(const uvw::ErrorEvent& error)
{
    m_callback(error, m_remote, m_local);
    delete this;
}
