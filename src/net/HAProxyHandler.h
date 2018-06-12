#pragma once
#include <stdint.h>
#include <vector>
#include "deps/uvw/uvw.hpp"

// Maximum size of the HAProxy header, per documentation:
#define HAPROXY_HEADER_MAX 107
// Minimum size of the HAProxy header:
#define HAPROXY_HEADER_MIN 16

class HAProxyHandler
{
    private:
        // Data members.
        std::vector<uint8_t> m_data_buf;
        std::vector<uint8_t> m_tlv_buf;
        
        uvw::Addr m_local;
        uvw::Addr m_remote;

        bool m_is_local = false;

        bool m_has_error = false;
        uv_errno_t m_error_code;

        void set_error(uv_errno_t ec);
        size_t parse_proxy_block();

        size_t parse_v1_block();
        size_t parse_v2_block();
    public:
        size_t consume(const uint8_t* buffer, size_t length); 
        
        inline uvw::Addr get_local() const
        {
            return m_local;
        }

        inline uvw::Addr get_remote() const
        {
            return m_remote;
        }

        inline const std::vector<uint8_t>& get_tlvs() const
        {
            return m_tlv_buf;
        }

        inline bool has_tlvs() const
        {
            return m_tlv_buf.size() > 0;
        }

        inline bool is_local() const
        {
            return m_is_local;
        }

        inline bool has_error() const
        {
            return m_has_error;
        }

        inline uv_errno_t get_error() const
        {
            return m_error_code;
        }
};
