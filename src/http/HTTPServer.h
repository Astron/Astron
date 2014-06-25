#pragma once

#include "core/global.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#define MAX_HTTP_DATA_LENGTH 4096

typedef std::tuple <int, std::string, std::string> errorMimeResponse_t;

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
        
        
        boost::asio::ip::tcp::socket m_socket;
		std::string m_ipAddress;
		
        char m_request[MAX_HTTP_DATA_LENGTH];
};

class HTTPServer
{
    public:
        HTTPServer(std::string str_ip, std::string str_port, std::string web_path);
        ~HTTPServer();
        
        // connection loop, calls handle_accept
        void start_accept();
        
        // client connected callback
        void handle_accept(HTTPConnection::pointer new_connection, 
                            const boost::system::error_code &ec);
                            
                            
        static errorMimeResponse_t handleRequest(std::string requestName, std::map <std::string, std::string> params, std::string method, std::string ipAddress);
        static errorMimeResponse_t handleAppPage(std::string url, std::string ipAddress);
        
        static errorMimeResponse_t serve404(std::string info = "");
        static errorMimeResponse_t serveFile(std::string mimeType, std::string content);    
        static errorMimeResponse_t serveRequest(int error, std::string request); 
        static errorMimeResponse_t serveFrozenPage(std::string url, std::string mimeType);  
        
        static std::string mimeFromExt(std::string ext);
		static bool detectDirectoryTraversal(std::string url);
		
                            
        static std::string m_webPath;
                            
    private:
        boost::asio::ip::tcp::acceptor *m_acceptor;
};

