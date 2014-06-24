#pragma once

#include <boost/asio.hpp>

class HTTPServer
{
    public:
        HTTPServer(std::string str_ip, std::string str_port);
        ~HTTPServer();
        
        // connection loop, calls handle_accept
        void start_accept();
        
        // client connected callback
        void handle_accept(boost::asio::ip::tcp::socket *socket, 
                            const boost::system::error_code &ec);
                            
    private:
        boost::asio::ip::tcp::acceptor *m_acceptor;
};