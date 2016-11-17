#include "HAProxyHandler.h"
#include <boost/bind.hpp>
#include <cstring>

using namespace boost::asio::ip;

static const uint8_t haproxy_signature_v1[] = {
    'P', 'R', 'O', 'X', 'Y', ' ', 'T', 'C', 'P'
};
static const uint8_t haproxy_signature_v2[] = {
    0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A
};

void HAProxyHandler::async_process(tcp::socket *socket, ProxyCallback &callback)
{
    HAProxyHandler *handler = new HAProxyHandler(socket, callback);
    handler->begin();
}

HAProxyHandler::HAProxyHandler(tcp::socket *socket, ProxyCallback &callback) :
    m_socket(socket), m_callback(callback)
{
    boost::system::error_code ec;
    m_remote = m_socket->remote_endpoint(ec);
    m_local = m_socket->local_endpoint(ec);
}

HAProxyHandler::~HAProxyHandler()
{
    if(m_body_buf != nullptr) {
        delete [] m_body_buf;
    }
}

void HAProxyHandler::begin()
{
    async_read(*m_socket, boost::asio::buffer(m_header_buf, HAPROXY_HEADER_MIN),
               boost::bind(&HAProxyHandler::handle_header, this,
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred));
}

void HAProxyHandler::handle_header(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec) {
        // Something happened, abort!
        finish(ec);
        return;
    }

    if(bytes_transferred != HAPROXY_HEADER_MIN) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe,
                                        boost::system::system_category());
        finish(epipe);
        return;
    }

    if(!memcmp(m_header_buf, haproxy_signature_v2, sizeof(haproxy_signature_v2))) {
        // Yay, it's a binary proxy header! These are easy to process:
        m_body_len = (m_header_buf[14] << 8) | m_header_buf[15];
        m_body_buf = new uint8_t[m_body_len];
        async_read(*m_socket, boost::asio::buffer(m_body_buf, m_body_len),
                   boost::bind(&HAProxyHandler::handle_v2, this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    } else if(!memcmp(m_header_buf, haproxy_signature_v1, sizeof(haproxy_signature_v1))) {
        // This is the ASCII version (begins with "PROXY ", terminated by
        // "\r\n"). These are harder to process, since we don't know how much
        // proxy data there is in advance, and we can't risk reading even a
        // single byte past the end of the "\r\n" because that's going to be
        // application-layer data and we have no facility for putting it back
        // into the read buffer after we take it out.

        // This function deals with it.
        handle_v1(ec, bytes_transferred);
    } else {
        // Couldn't recognize any proxy header.
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
    }
}

void HAProxyHandler::handle_v2(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec) {
        // Something happened, abort!
        finish(ec);
        return;
    }

    if(bytes_transferred != m_body_len) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe,
                                        boost::system::system_category());
        finish(epipe);
        return;
    }

    parse_v2();
}

void HAProxyHandler::parse_v2()
{
    boost::system::error_code no_error;

    int version = (m_header_buf[12] >> 4) & 0xF;
    int command = (m_header_buf[12]) & 0xF;
    int family  = (m_header_buf[13] >> 4) & 0xF;
    int transp  = (m_header_buf[13]) & 0xF;

    if(version != 2) {
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
        return;
    }

    if(family != 0x1 && family != 0x2) {
        boost::system::error_code eafnosupport(boost::system::errc::errc_t::address_family_not_supported,
                                               boost::system::system_category());
        finish(eafnosupport);
        return;
    }

    if(transp != 0x1) {
        boost::system::error_code eprotonosupport(boost::system::errc::errc_t::protocol_not_supported,
                boost::system::system_category());
        finish(eprotonosupport);
        return;
    }

    if(command == 0x0) {
        // LOCAL, ignore the body:
        finish(no_error);
        return;
    }

    if(command != 0x1) {
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
        return;
    }

    if((family == 0x1 && m_body_len < 12) ||
       (family == 0x2 && m_body_len < 36)) {
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
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

        remote_address = remote_v4;
        local_address = local_v4;

        port_offset = 8;
    } else {
        address_v6::bytes_type address_bytes;

        memcpy(address_bytes.data(), &m_body_buf[0], 16);
        address_v6 remote_v6(address_bytes);

        memcpy(address_bytes.data(), &m_body_buf[16], 16);
        address_v6 local_v6(address_bytes);

        remote_address = remote_v6;
        local_address = local_v6;

        port_offset = 32;
    }

    int remote_port = m_body_buf[port_offset] << 8 | m_body_buf[port_offset + 1];
    int local_port = m_body_buf[port_offset + 2] << 8 | m_body_buf[port_offset + 3];

    m_remote = tcp::endpoint(remote_address, remote_port);
    m_local = tcp::endpoint(local_address, local_port);
    finish(no_error);
}

void HAProxyHandler::handle_v1(const boost::system::error_code &ec, size_t bytes_transferred)
{
    if(ec) {
        // Something happened, abort!
        finish(ec);
        return;
    }

    if(bytes_transferred == 0) {
        boost::system::error_code epipe(boost::system::errc::errc_t::broken_pipe,
                                        boost::system::system_category());
        finish(epipe);
        return;
    }

    m_header_len += bytes_transferred;

    // See if we got a '\n', indicating the end of the proxy header:
    if(memchr(m_header_buf, '\n', m_header_len) != nullptr) {
        parse_v1();
        return;
    }

    if(m_header_len >= HAPROXY_HEADER_MAX) {
        boost::system::error_code eoverflow(boost::system::errc::errc_t::value_too_large,
                                            boost::system::system_category());
        finish(eoverflow);
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

    async_read(*m_socket,
               boost::asio::buffer(&m_header_buf[m_header_len],
                                   std::min(read_size, remaining_size)),
               boost::bind(&HAProxyHandler::handle_v1, this,
                           boost::asio::placeholders::error,
                           boost::asio::placeholders::bytes_transferred));
}

void HAProxyHandler::parse_v1()
{
    if((m_header_buf[m_header_len - 2] != '\r') ||
       (m_header_buf[m_header_len - 1] != '\n')) {
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
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
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
        return;
    }

    address remote_address;
    address local_address;

    if(!strcmp(tcp, "TCP4")) {
        remote_address = address_v4::from_string(srcip);
        local_address = address_v4::from_string(dstip);
    } else if(!strcmp(tcp, "TCP6")) {
        remote_address = address_v6::from_string(srcip);
        local_address = address_v6::from_string(dstip);
    } else {
        boost::system::error_code eproto(boost::system::errc::errc_t::protocol_error,
                                         boost::system::system_category());
        finish(eproto);
        return;
    }

    boost::system::error_code no_error;
    m_remote = tcp::endpoint(remote_address, atoi(srcport));
    m_local = tcp::endpoint(local_address, atoi(dstport));
    finish(no_error);
}

void HAProxyHandler::finish(const boost::system::error_code &ec)
{
    m_callback(ec, m_remote, m_local);
    delete this;
}
