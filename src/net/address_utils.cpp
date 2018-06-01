#include "address_utils.h"

#include <algorithm>
#include <utility>

#include "deps/uvw/uvw.hpp"
#include "deps/uvw/uvw/util.hpp"
#include <uv.h>

static bool split_port(std::string &ip, uint16_t &port)
{
    size_t last_colon = ip.rfind(':');
    if(last_colon == std::string::npos) {
        return true;
    }

    // If the input contains *multiple* colons, it's an IPv6 address.
    // We ignore these unless the IPv6 address is bracketed and the port
    // specification occurs outside of the brackets.
    // (e.g. "[::]:1234")
    if(std::count(ip.begin(), ip.end(), ':') > 1) {
        // Yep, we're assuming IPv6. Let's see if the last colon has a
        // ']' before it.
        // Note that this still can lead to weird inputs getting
        // through, but that'll get caught by a later parsing step. :)
        if(ip[last_colon - 1] != ']') {
            return true;
        }
    }

    try {
        port = std::stoi(ip.substr(last_colon + 1));
        ip = ip.substr(0, last_colon);
    } catch(const std::invalid_argument&) {
        return false;
    }

    return true;
}

static std::pair<bool, uvw::Addr> parse_address(const std::string &ip, uint16_t port)
{
    if((ip.find(':') != std::string::npos) || (ip[0] == '[' && ip[ip.length() - 1] == ']')) {
        sockaddr_in6 sockaddr;
        size_t opening_bracket = ip.find('[');
        size_t closing_bracket = ip.find(']');
        if((opening_bracket != std::string::npos && closing_bracket == std::string::npos) ||
           (closing_bracket != std::string::npos && opening_bracket == std::string::npos)) {
            // Unbalanced set of brackets, this is an invalid address.
            return std::pair<bool, uvw::Addr>(false, uvw::Addr());
        }

        // Strip brackets from IPv6 address (if they exist). libuv won't accept them otherwise.
        std::string unbracketed_addr = ip;
        unbracketed_addr.erase(std::remove(unbracketed_addr.begin(), unbracketed_addr.end(), '['), unbracketed_addr.end());
        unbracketed_addr.erase(std::remove(unbracketed_addr.begin(), unbracketed_addr.end(), ']'), unbracketed_addr.end());
        if(uv_ip6_addr(unbracketed_addr.c_str(), port, &sockaddr) == 0) {
            return std::pair<bool, uvw::Addr>(true, uvw::details::address<uvw::IPv6>(&sockaddr));
        }
        else {
            return std::pair<bool, uvw::Addr>(false, uvw::Addr());
        }
    } else {
        sockaddr_in sockaddr;
        if (uv_ip4_addr(ip.c_str(), port, &sockaddr) == 0) {
            uvw::Addr addr = uvw::details::address<uvw::IPv4>(&sockaddr);
            return std::pair<bool, uvw::Addr>(true, addr);
        }
        else {
            return std::pair<bool, uvw::Addr>(false, uvw::Addr());
        }
    }
}

static bool validate_hostname(const std::string &hostname)
{
    // Standard hostname rules (a la RFC1123):
    // - Must consist of only the characters: A-Z a-z 0-9 . -
    // - Each dot-separated section must contain at least one character.
    // - The - character may not appear twice.
    // - Hostname sections may not begin or end with a -
    //
    // In other words, we must not have a "..", "-.", or ".-" anywhere.
    // We also have to verify that the beginning of the hostname is not a
    // "-" or "." and that the end is not a "-".
    // Everything else is accepted as a valid hostname.

    for(auto it = hostname.begin(); it != hostname.end(); ++it) {
        if(*it != '.' && *it != '-' && !isalnum(*it)) {
            return false;
        }
    }

    if(hostname.find("..") != std::string::npos ||
       hostname.find("-.") != std::string::npos ||
       hostname.find(".-") != std::string::npos) {
        return false;
    }

    if(hostname[0] == '-' || hostname[0] == '.' || hostname[0] == '\0') {
        return false;
    }

    if(hostname[hostname.length() - 1] == '-') {
        return false;
    }

    return true;
}

bool is_valid_address(const std::string &hostspec)
{
    std::string host = hostspec;
    uint16_t port = 0;

    if(!split_port(host, port)) {
        return false;
    }

    auto result = parse_address(host, port);

    if(result.first) {
        return true;
    } else {
        return validate_hostname(host);
    }
}

std::vector<uvw::Addr> resolve_address(const std::string &hostspec, uint16_t port, const std::shared_ptr<uvw::Loop> &loop)
{
    #ifdef _WIN32
        // Windows seems to consistently do the wrong thing here:
        // For nodeAddrInfoSync requests, libuv always returns an ai_socktype of 0.
        // Ergo, this hack is necessary for DNS resolutions on Windows.
        const int socktype = 0;
    #else
        const int socktype = SOCK_STREAM;
    #endif

    std::vector<uvw::Addr> ret;

    std::string host = hostspec;

    if(!split_port(host, port)) {
        return ret;
    }

    auto result = parse_address(host, port);
    if(result.first) {
        ret.push_back(result.second);
    } else {
        std::shared_ptr<uvw::GetAddrInfoReq> request = loop->resource<uvw::GetAddrInfoReq>();
        auto results = request->nodeAddrInfoSync(host);
        if (results.first) {
            addrinfo* addrinfo = results.second.get();
            while (addrinfo != nullptr) {
                if (addrinfo->ai_family == AF_INET && addrinfo->ai_socktype == socktype) {
                    sockaddr_in* sockaddr = reinterpret_cast<sockaddr_in*>(addrinfo->ai_addr);
                    uvw::Addr addr = uvw::details::address<uvw::IPv4>(sockaddr);
                    addr.port = port;
                    ret.push_back(addr);
                }
                else if (addrinfo->ai_family == AF_INET6 && addrinfo->ai_socktype == socktype) {
                    sockaddr_in6* sockaddr = reinterpret_cast<sockaddr_in6*>(addrinfo->ai_addr);
                    uvw::Addr addr = uvw::details::address<uvw::IPv6>(sockaddr);
                    addr.port = port;
                    ret.push_back(addr);
                }

                addrinfo = addrinfo->ai_next;
            }

        }
    }

    return ret;
}
