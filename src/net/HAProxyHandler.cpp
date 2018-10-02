#include "HAProxyHandler.h"
#include <cstring>
#include <cassert>

static const uint8_t haproxy_signature_v1[] = {
    'P', 'R', 'O', 'X', 'Y', ' ', 'T', 'C', 'P'
};

static const uint8_t haproxy_signature_v2[] = {
    0x0D, 0x0A, 0x0D, 0x0A, 0x00, 0x0D, 0x0A, 0x51, 0x55, 0x49, 0x54, 0x0A
};

void HAProxyHandler::set_error(uv_errno_t ec)
{   
    m_has_error = true;
    m_error_code = ec;
}

size_t HAProxyHandler::consume(const uint8_t* buffer, size_t length)
{ 
    if(m_has_error) {
        // Early-out if an error has been set.
        return 0;
    }

    size_t prev_size = m_data_buf.size();

    m_data_buf.insert(m_data_buf.end(), buffer, buffer + length);

    size_t hdr_size = parse_proxy_block();

    if(hdr_size == 0) {
        // Not a valid PROXY block.
        set_error(UV_EPROTO);
        return 0;
    }

    if(hdr_size < m_data_buf.size()) {
        // Returns how many bytes we consumed if the proxy block is over capacity. 
        return hdr_size - prev_size;
    }
    else if(hdr_size == m_data_buf.size()) {
        // Consumed exactly as many bytes as we were given in the call for the header.
        return 0;
    }

    return length;
}

size_t HAProxyHandler::parse_proxy_block()
{
    if(m_data_buf.size() < HAPROXY_HEADER_MIN) {
        // We need to have at least HAPROXY_HEADER_MIN bytes in our buffer to estimate size.
        return HAPROXY_HEADER_MIN;
    }

    if(!memcmp(&m_data_buf[0], haproxy_signature_v2, sizeof(haproxy_signature_v2))) {
        // PROXY v2 header: We can easily determine the header size with just HAPROXY_HEADER_MIN bytes.
        return parse_v2_block();
    } else if(!memcmp(&m_data_buf[0], haproxy_signature_v1, sizeof(haproxy_signature_v1))) {
        // PROXY v1 header: This is the human-readable (ASCII) form of the HAProxy header.
        // We need to keep returning m_data_buf.size() + 2 (or m_data_buf.size() + 1 if a CR character is already at the end of the buffer)
        return parse_v1_block();
    }

    // The data passed doesn't reflect that of a valid PROXYv1/PROXYv2 header, we should never hit this.
    // Signal this back to consume()
    return 0;
}

size_t HAProxyHandler::parse_v2_block()
{
    assert(!(m_data_buf.size() < HAPROXY_HEADER_MIN));

    size_t body_len = (m_data_buf[14] << 8) | m_data_buf[15];
    size_t total_len = HAPROXY_HEADER_MIN + body_len;

    if(m_data_buf.size() < total_len) {
        // We don't have enough bytes for the PROXYv2 block yet.
        return total_len;
    }

    int version = (m_data_buf[12] >> 4) & 0xF;
    int command = (m_data_buf[12]) & 0xF;
    int family = (m_data_buf[13] >> 4) & 0xF;
    int transp = (m_data_buf[13]) & 0xF;

    if(version != 2) {
        set_error(UV_EPROTO);
        return 0;
    }

    if(family != 0x1 && family != 0x2) {
        set_error(UV_EAFNOSUPPORT);
        return 0;
    }

    if(transp != 0x1) {
        set_error(UV_EPROTONOSUPPORT);
        return 0;
    }

    if(command == 0x0) {
        // LOCAL, ignore the body:
        m_is_local = true;
        return total_len;
    }

    if(command != 0x1) {
        set_error(UV_EPROTO);
        return 0;
    }

    if((family == 0x1 && body_len < 12) ||
       (family == 0x2 && body_len < 36)) {
        set_error(UV_EPROTO);
        return 0;
    }

    char* remote_address = NULL;
    char* local_address = NULL;
    int port_offset = HAPROXY_HEADER_MIN;

    if(family == 0x1) {
        remote_address = new char[INET_ADDRSTRLEN];
        local_address = new char[INET_ADDRSTRLEN];
        uv_inet_ntop(AF_INET, &m_data_buf[HAPROXY_HEADER_MIN], remote_address, INET_ADDRSTRLEN);
        uv_inet_ntop(AF_INET, &m_data_buf[HAPROXY_HEADER_MIN + 4], local_address, INET_ADDRSTRLEN);
        port_offset += 8;
    } else {
        remote_address = new char[INET6_ADDRSTRLEN];
        local_address = new char[INET6_ADDRSTRLEN];
        uv_inet_ntop(AF_INET6, &m_data_buf[HAPROXY_HEADER_MIN], remote_address, INET6_ADDRSTRLEN);
        uv_inet_ntop(AF_INET6, &m_data_buf[HAPROXY_HEADER_MIN + 16], local_address, INET6_ADDRSTRLEN);
        port_offset += 32;
    }

    int remote_port = m_data_buf[port_offset] << 8 | m_data_buf[port_offset + 1];
    int local_port = m_data_buf[port_offset + 2] << 8 | m_data_buf[port_offset + 3];

    m_remote.ip = std::string(remote_address);
    m_remote.port = remote_port;
    m_local.ip =  std::string(local_address);
    m_local.port = local_port;

    delete[] remote_address;
    delete[] local_address;

    // Store TLVs into our own internal buffer, to be copied by NetworkClient for CA-sided consumption.
    size_t tlv_off = port_offset + 4;
    ssize_t tlv_size = (total_len - tlv_off);
    if(tlv_size > 0) {
        m_tlv_buf = std::vector<uint8_t>(&m_data_buf[tlv_off], &m_data_buf[tlv_off + tlv_size]);
    }

    return total_len;
}

size_t HAProxyHandler::parse_v1_block()
{
    assert(!(m_data_buf.size() < HAPROXY_HEADER_MIN));

    size_t capped_length = m_data_buf.size() > HAPROXY_HEADER_MAX ? HAPROXY_HEADER_MAX : m_data_buf.size();

    char* cr_chr = (char*)memchr(&m_data_buf[0], '\r', capped_length);

    if(cr_chr == NULL) {
        if(m_data_buf.size() >= HAPROXY_HEADER_MAX - 1) {
            // We should have a CR by now, if we don't we're not dealing with a valid PROXYv1 block as per the spec.
            set_error(UV_EPROTO);
            return 0;
        }

        // We need *at least* 2 more bytes for the header to be complete.
        return m_data_buf.size() + 2;
    }

    size_t cr_off = (cr_chr - (char*)&m_data_buf[0]) + 1;

    if(cr_off > HAPROXY_HEADER_MAX - 1) {
        // If the CR is after the 106th character in our buffer, we probably stepped into application-layer data.
        set_error(UV_EPROTO);
        return 0;
    }

    char* lf_chr = (char*)memchr(&m_data_buf[0], '\n', capped_length);
    if(lf_chr == NULL || lf_chr != cr_chr + 1) {
        if(m_data_buf.size() >= HAPROXY_HEADER_MAX) {
            // We should have an LF character located after our CR by now.
            set_error(UV_EPROTO);
            return 0;
        }

        // Either there is no line-feed character, or for whatever reason it isn't located after a CR.
        // Only expect one more byte.
        return m_data_buf.size() + 1;
    }

    // (header_end_ptr - header_beg_ptr) + 1 is our final PROXYv1 header size.
    size_t hdr_size = (lf_chr - (char*)&m_data_buf[0]) + 1;

    if(hdr_size > HAPROXY_HEADER_MAX) {
        // Our CRLF is after the maximum bounds of the expected HAProxy header size.
        set_error(UV_EPROTO);
        return 0;
    }

    // Our header obviously cannot be larger than the buffer it originates from.
    assert(!(m_data_buf.size() < hdr_size));

    // Scratch-buffer for strtok() related operations.
    char* header_buf = new char[hdr_size + 1];
    memcpy(header_buf, &m_data_buf[0], hdr_size);
    header_buf[hdr_size] = '\0';

    // Ignore the return value of the first strtok() call: It's just gonna be "PROXY"
    strtok(header_buf, " ");
    const char *tcp = strtok(NULL, " ");
    const char *srcip = strtok(NULL, " ");
    const char *dstip = strtok(NULL, " ");
    const char *srcport = strtok(NULL, " ");
    const char *dstport = strtok(NULL, "\r");

    if(dstport == nullptr) {
        set_error(UV_EPROTO);
        return 0;
    }

    if(strcmp(tcp, "TCP4") != 0 && strcmp(tcp, "TCP6") != 0) {
        set_error(UV_EPROTO);
        return 0;
    }

    m_remote.ip = std::string(srcip);
    m_remote.port = atoi(srcport);
    m_local.ip =  std::string(dstip);
    m_local.port = atoi(dstport);

    delete[] header_buf;

    return hdr_size;
}

