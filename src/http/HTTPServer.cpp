#include "HTTPServer.h"

#include "core/global.h"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using namespace std;

HTTPServer::HTTPServer(string str_ip, string str_port)
{    
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(str_ip, str_port);
    tcp::resolver::iterator it = resolver.resolve(query);
    
    m_acceptor = new tcp::acceptor(io_service, *it, true);
        
    start_accept();
}

HTTPServer::~HTTPServer() {
    
}

void HTTPServer::start_accept()
{
    tcp::socket *socket = new tcp::socket(io_service);
    tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(*socket, boost::bind(&HTTPServer::handle_accept,
	                         this, socket, boost::asio::placeholders::error));
}

void HTTPServer::handle_accept(tcp::socket *socket, const boost::system::error_code& /*ec*/)
{
	boost::asio::ip::tcp::endpoint remote;
	try
	{
		remote = socket->remote_endpoint();
	}
	catch (exception&)
	{
		start_accept();
		return;
	}
    
    cout << "HTTP Server incoming connection: " << remote.address().to_string() << "\n";
    
    char* content = "<i>We're sorry, but Astron Web Administration is currently under construction. Nothing to see here!</i>";
    
    char data[2048];
    sprintf(data, "HTTP/1.1 200 OK\nServer: Astron Web Administration\nContent-Type: text/html; charset=UTF-8;\nContent-Length: %d\nConnection: close\n\n%s\n",
        strlen(content),
        content); 
    
    socket->write_some(boost::asio::buffer(data, strlen(data)));

	start_accept();
}
