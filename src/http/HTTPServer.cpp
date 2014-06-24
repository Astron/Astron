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
    
	m_acceptor->async_accept(new_connection->socket(), 
                            boost::bind(&HTTPServer::handle_accept,
	                         this, new_connection, boost::asio::placeholders::error));
}

void HTTPServer::handle_accept(HTTPConnection::pointer new_connection,           
                            const boost::system::error_code& ec)
{
    if(!ec)
    {
        new_connection->start();
    }
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
    
    std::vector<std::string> urlVsData;
    boost::split(urlVsData, url, boost::is_any_of("?"));
    
    std::vector<std::string> urlParts;
    boost::split(urlParts, urlVsData[0], boost::is_any_of("/"));
    
    std::vector<std::string> dataParts;
    
    if(urlVsData.size() > 1) {
        boost::split(dataParts, urlVsData[1], boost::is_any_of("&"));
    }
    
    /*stringstream content;
    content << "<h1><i>We're sorry, but Astron Web Administration is currently under construction. Nothing to see here!</i></h1><br/>"
                            << "....<h2>Alternately, for developers and creeps stalking Astron's pull requests, here's some info:</h2></br>"
                            << "<h3>Request Type: </h3> " << requestType << "<br/>"
                            << "<h3> URL: </h3> " << url << "<br/>"
                            << "<h3> Full Request: </h3> <p>" << m_request << "</p>";*/
    
    std::string content = "Default";
    std::string mimeType = "text/plain";
    
    if(urlParts[1] == "app") {
        content = handleAppPage(url);
        mimeType = "text/html";
    } else if(urlParts[1] == "request" && urlVsData.size() > 1) {
        content = handleRequest(urlParts[2], dataParts, requestType);
        mimeType = "text/plain";
    }
    
    stringstream s;
    s << "HTTP/1.1 200 OK\nServer: Astron Web Administration\nContent-Type: " << mimeType << "\nContent-Length: "
         << content.length() << "\nConnection: close\n\n" 
         << content << "\n";
    
    boost::asio::async_write(m_socket, boost::asio::buffer(s.str()), 
        boost::bind(&HTTPConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

std::string HTTPConnection::handleRequest(std::string requestName, std::vector<std::string> params, std::string method) {
    cout << "Request " << requestName << " made: argument 1 " << params.front() << "\n";
    
    return "{\"success\" : \"1\"}";
}

std::string HTTPConnection::handleAppPage(std::string url) {
    cout << "Serve page " << url << "\n";
    
    stringstream s;
    s << "<h1><i>We're sorry, but Astron Web Administration is currently under construction. Nothing to see here!</i></h1><br/>"
                            << "This is an app page <br/>"
                            << "In the future, this will point to: "
                            << url;
    
    return s.str();
}