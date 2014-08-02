#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

class NetworkConnector
{
  public:
    NetworkConnector(boost::asio::io_service &io_service);

    // Parses the string "address" and connects to it. If no port is specified
    // as part of the address, it will use default_port.
    // The return value will either be a freshly-allocated socket, or NULL.
    // If the return value is NULL, the error code will be set to indicate
    // the reason that the connect failed.
    tcp::socket *connect(const std::string &address, unsigned int default_port,
                         boost::system::error_code &ec);
    ssl::stream<tcp::socket> *connect(const std::string &address, unsigned int default_port,
                                      ssl::context *ctx, boost::system::error_code &ec);
  private:
    boost::asio::io_service &m_io_service;
};
