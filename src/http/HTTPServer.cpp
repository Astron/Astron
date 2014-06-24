#include "HTTPServer.h"

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
    HTTPConnection::pointer new_connection = HTTPConnection::create(io_service);
    
   // tcp::socket *socket = new tcp::socket(io_service);
    //tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(new_connection->socket(), 
                            boost::bind(&HTTPServer::handle_accept,
	                         this, new_connection, boost::asio::placeholders::error));
}

void HTTPServer::handle_accept(HTTPConnection::pointer new_connection,           
                            const boost::system::error_code& ec)
{
    if(!ec) {
        new_connection->start();
    }
    
	/*boost::asio::ip::tcp::endpoint remote;
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
        
    socket->write_some(boost::asio::buffer(data, strlen(data)));*/

	start_accept();
}

void HTTPConnection::start ()
{
    m_socket.async_read_some( boost::asio::buffer(m_request, MAX_HTTP_DATA_LENGTH), 
                               boost::bind(&HTTPConnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
}

void HTTPConnection::handle_read(const boost::system::error_code& /* ec */,
                    size_t /* bytes_transferred*/)
{
    
    std::vector<std::string> headers;
    boost::split(headers, m_request, boost::is_any_of("\r\n"));
    
    std::vector<std::string> topLine;
    boost::split(topLine, headers[0], boost::is_any_of(" "));
    
    string requestType = topLine[0];
    string url = topLine[1];
    
    stringstream content;
    content << "<h1><i>We're sorry, but Astron Web Administration is currently under construction. Nothing to see here!</i></h1><br/>"
                            << "....<h2>Alternately, for developers and creeps stalking Astron's pull requests, here's some info:</h2></br>"
                            << "<h3>Request Type: </h3> " << requestType << "<br/>"
                            << "<h3> URL: </h3> " << url << "<br/>"
                            << "<h3> Full Request: </h3> <p>" << m_request << "</p>";
    
    stringstream s;
    s << "HTTP/1.1 200 OK\nServer: Astron Web Administration\nContent-Type: text/html; charset=UTF-8;\nContent-Length: "
         << content.str().length() << "\nConnection: close\n\n" 
         << content.str() << "\n";
    
    boost::asio::async_write(m_socket, boost::asio::buffer(s.str()), 
        boost::bind(&HTTPConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    
}


