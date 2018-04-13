#pragma once
#include <stdint.h>
#include <vector>
#include <deps/uvw/uvw.hpp>

// Maximum size of the HAProxy header, per documentation:
#define HAPROXY_HEADER_MAX 107
// Minimum size of the HAProxy header:
#define HAPROXY_HEADER_MIN 16

enum HAProxyHandler_State
{
    HAPROXY_HDR_PARSE,
    HAPROXY_BODY_V2_PARSE,
    HAPROXY_FIN
};

class HAProxyHandler
{
    private:
        // Data members.
        std::vector<uint8_t> m_data_buf;
        uint8_t m_header_buf[HAPROXY_HEADER_MAX + 1];
        size_t m_header_len = 0;
        uint8_t* m_body_buf;
        size_t m_body_len = 0;

        HAProxyHandler_State m_state;
        uvw::Addr m_local;
        uvw::Addr m_remote;

        bool m_has_error;
        uv_errno_t m_error_code;

        void run_fsm();
        void set_state(HAProxyHandler_State state);
        void set_error(uv_errno_t ec);

        void parse_header();
        void parse_body_v2();
    public:
        HAProxyHandler();
        void consume(const uint8_t* buffer, size_t length);
        const std::vector<uint8_t>& get_buffer() const;
        bool is_done() const;
        uvw::Addr get_local() const;
        uvw::Addr get_remote() const;
        bool has_error() const;
        uv_errno_t get_error() const;
};
