#pragma once

#include "core/global.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/algorithm/string.hpp>

#define MAX_HTTP_DATA_LENGTH 4096

class HTTPConnection 
    : public boost::enable_shared_from_this<HTTPConnection>
{
    public:
        typedef boost::shared_ptr<HTTPConnection> pointer;
        
        static pointer create(boost::asio::io_service& io_service)
        {
          return pointer(new HTTPConnection(io_service));
        }

        boost::asio::ip::tcp::socket& socket()
        {
          return m_socket;
        }
        
        void start ();
        
    private:
        HTTPConnection(boost::asio::io_service& io_service)
          : m_socket(io_service)
        {
            
        }
        
        void handle_read(const boost::system::error_code& /* ec */,
                            size_t /* bytes_transferred*/);
        
        void handle_write(const boost::system::error_code& /* ec */,
                            size_t /* bytes_transferred*/)                
        {
            
        }
        
        std::string handleRequest(std::string requestName, std::map <std::string, std::string> params, std::string method);
        std::string handleAppPage(std::string url);
        
        boost::asio::ip::tcp::socket m_socket;
        char m_request[MAX_HTTP_DATA_LENGTH];
};

class HTTPServer
{
    public:
        HTTPServer(std::string str_ip, std::string str_port);
        ~HTTPServer();
        
        // connection loop, calls handle_accept
        void start_accept();
        
        // client connected callback
        void handle_accept(HTTPConnection::pointer new_connection, 
                            const boost::system::error_code &ec);
                            
    private:
        boost::asio::ip::tcp::acceptor *m_acceptor;
};

