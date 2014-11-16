#include <boost/asio.hpp>

using namespace boost::asio::ip;

bool is_valid_address(const std::string &hostspec);
std::vector<tcp::endpoint> resolve_address(
    const std::string &hostspec, uint16_t port,
    boost::asio::io_service &io_service, boost::system::error_code &ec);
