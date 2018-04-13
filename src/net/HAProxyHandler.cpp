#include "HAProxyHandler.h"
#include <cstring>
#include <cassert>

static const uint8_t haproxy_signature_v1[] = {
    'P', 'R', 'O', 'X', 'Y', ' ', 'T', 'C', 'P'
};

static const uint8_t haproxy_signature_v2[] = {
    0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A
};

HAProxyHandler::HAProxyHandler() : m_header_len(0), m_body_len(0), m_state(HAPROXY_HDR_PARSE),
                                   m_has_error(false), m_error_code((uv_errno_t)0)
{
}

void HAProxyHandler::run_fsm()
{
    switch(m_state) {
        case HAPROXY_HDR_PARSE:
            parse_header();
            break;
        case HAPROXY_BODY_V2_PARSE:
            parse_body_v2();
            break;
        default:
            break;
    }
}

void HAProxyHandler::set_state(HAProxyHandler_State state)
{
    m_state = state;

    run_fsm();
}

void HAProxyHandler::set_error(uv_errno_t ec)
{   
    m_has_error = true;
    m_error_code = ec;
    
    set_state(HAPROXY_FIN);
}

void HAProxyHandler::consume(const uint8_t* buffer, size_t length)
{
    if(m_state == HAPROXY_FIN) {
        // Any further invocations of consume will black-hole any data passed if in a FIN state.
        // It is the responsibility of the invoker to check the state of the FSM prior to calling consume.
        return;
    }

    // We write any data passed to consume() into our own internal buffer.
    // It is the responsibility of each FSM function to consume as many bytes as it deems fit, and remove them from m_data_buf in the process.
    // Making sure that the FSM state handlers leave the data buffer at a consistent state is critical:
    // The application layer relies on it when the FSM hits a FIN state in order to re-insert any over-read bytes off it for its own processing.
    // On that note, it is also the responsibility of the NetworkClient to retrieve any over-read bytes at the end of the HAProxyHandler's operation.
    m_data_buf.insert(m_data_buf.end(), buffer, buffer + length);

    // Run the actual FSM.
    run_fsm();
}

uvw::Addr HAProxyHandler::get_remote() const
{
    return m_remote;
}

uvw::Addr HAProxyHandler::get_local() const
{
    return m_local;
}

const std::vector<uint8_t>& HAProxyHandler::get_buffer() const
{
    return m_data_buf;
}

bool HAProxyHandler::is_done() const
{
    return m_state == HAPROXY_FIN;
}

bool HAProxyHandler::has_error() const
{
    return m_has_error;
}

uv_errno_t HAProxyHandler::get_error() const
{
    return m_error_code;
}

void HAProxyHandler::parse_header()
{
    // Make sure we're not entering from an invalid state.
    assert(m_state == HAPROXY_HDR_PARSE);

    if(m_data_buf.size() < HAPROXY_HEADER_MIN) {
        // Not enough data to do anything here.
        return;
    }

    if(!memcmp(&m_data_buf.front(), haproxy_signature_v2, sizeof(haproxy_signature_v2))) {
        // We're dealing with a PROXY v2 header. This is far easier for us to process.
        // We copy the header over to m_header_buf and clear m_data_buf of any bytes belonging to the header.
        m_header_len = HAPROXY_HEADER_MIN;

        memcpy(&m_header_buf, &m_data_buf.front(), m_header_len);
        m_body_len = (m_header_buf[14] << 8) | m_header_buf[15];
        m_body_buf = new uint8_t[m_body_len];

        size_t overread_size = m_data_buf.size() - m_header_len;

        if(overread_size > 0) {
            m_data_buf = std::vector<uint8_t>(&m_data_buf.front() + m_header_len, 
                                              &m_data_buf.front() + m_header_len + overread_size); 
        } else {
            m_data_buf = std::vector<uint8_t>();
        }

        set_state(HAPROXY_BODY_V2_PARSE);
    }
}

void HAProxyHandler::parse_body_v2()
{
    // Make sure we're not entering from an invalid state.
    assert(m_state == HAPROXY_BODY_V2_PARSE);

    // m_body_len can never be zero.
    assert(m_body_len != 0);

    // Similarly m_body_buf can never be NULL.
    assert(m_body_buf != nullptr);

    if(m_data_buf.size() < m_body_len) {
        // Not enough bytes consumed, return immediately.
        return;
    }

    memcpy(m_body_buf, &m_data_buf.front(), m_body_len);

    size_t overread_size = m_data_buf.size() - m_body_len;

    if(overread_size > 0) {
        m_data_buf = std::vector<uint8_t>(&m_data_buf.front() + m_body_len, 
                                          &m_data_buf.front() + m_body_len + overread_size);
    } else {
        m_data_buf = std::vector<uint8_t>();
    }

    int version = (m_header_buf[12] >> 4) & 0xF;
    int command = (m_header_buf[12]) & 0xF;
    int family  = (m_header_buf[13] >> 4) & 0xF;
    int transp  = (m_header_buf[13]) & 0xF;

    if(version != 2) {
        set_error(UV_EPROTO);
        return;
    }

    if(family != 0x1 && family != 0x2) {
        set_error(UV_EAFNOSUPPORT);
        return;
    }

    if(transp != 0x1) {
        set_error(UV_EPROTONOSUPPORT);
        return;
    }

    if(command == 0x0) {
        // LOCAL, ignore the body:
        set_state(HAPROXY_FIN);
        return;
    }

    if(command != 0x1) {
        set_error(UV_EPROTO);
        return;
    }

    if((family == 0x1 && m_body_len < 12) ||
       (family == 0x2 && m_body_len < 36)) {
        set_error(UV_EPROTO);
        return;
    }

    char* remote_address = NULL;
    char* local_address = NULL;
    int port_offset = 0;

    if(family == 0x1) {
        remote_address = new char[INET_ADDRSTRLEN];
        local_address = new char[INET_ADDRSTRLEN];
        uv_inet_ntop(AF_INET, &m_body_buf[0], remote_address, INET_ADDRSTRLEN);
        uv_inet_ntop(AF_INET, &m_body_buf[4], local_address, INET_ADDRSTRLEN);
        port_offset = 8;
    } else {
        remote_address = new char[INET6_ADDRSTRLEN];
        local_address = new char[INET6_ADDRSTRLEN];
        uv_inet_ntop(AF_INET6, &m_body_buf[0], remote_address, INET6_ADDRSTRLEN);
        uv_inet_ntop(AF_INET6, &m_body_buf[16], local_address, INET6_ADDRSTRLEN);
        port_offset = 32;
    }

    int remote_port = m_body_buf[port_offset] << 8 | m_body_buf[port_offset + 1];
    int local_port = m_body_buf[port_offset + 2] << 8 | m_body_buf[port_offset + 3];

    m_remote.ip = std::string(remote_address);
    m_remote.port = remote_port;
    m_local.ip =  std::string(local_address);
    m_local.port = local_port;

    delete[] remote_address;
    delete[] local_address;

    set_state(HAPROXY_FIN);
}
