#include "HTTPServer.h"

using boost::asio::ip::tcp;
using namespace std;

static string s_webPath;
 
HTTPServer::HTTPServer(string str_ip, string str_port, string web_path)
{    
    s_webPath = web_path;
    
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
    
    string mimeType = "text/plain";
    errorResponse_t resp = std::make_tuple(404, "Malformed request");
    
    if(urlParts[1] == "admin") {
        resp = HTTPServer::handleAppPage(url);
        mimeType = "text/html";
    } else if(urlParts[1] == "request" && urlVsData.size() > 1) {
        map <string, string> parameters;
        
        for(int i = 0; i < (int) dataParts.size(); ++i) {
            std::vector<std::string> dparts;
            boost::split(dparts, dataParts[i], boost::is_any_of("="));
            parameters[dparts[0]] = dparts[1];
        }
        
        resp = HTTPServer::handleRequest(urlParts[2], parameters, requestType);
        mimeType = "text/plain";
    }
    
    stringstream s;
    s << "HTTP/1.1 " << std::get<0>(resp) << " OK\nServer: Astron Web Administration\nContent-Type: " << mimeType << "\nContent-Length: "
         << std::get<1>(resp).length() << "\nConnection: close\n\n" 
         << std::get<1>(resp) << "\n";
    
    boost::asio::async_write(m_socket, boost::asio::buffer(s.str()), 
        boost::bind(&HTTPConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

errorResponse_t HTTPServer::handleRequest(std::string requestName, std::map <std::string, std::string> params, std::string method) {
                                                                 
    cout << "Request " << requestName << " made: password " << params["password"] << "\n";
    
    return std::make_tuple(200, "{\"success\" : \"1\"}");
}

errorResponse_t HTTPServer::handleAppPage(std::string url) {
    cout << "Serve page " << url << "\n";
    
    if(boost::find_first(url, "..") || boost::find_first(url, "~")) {
        cout << "LIKELY HACKING ATTEMPT" << "\n";
        return std::make_tuple(200, "<script>alert('Stop hacking');</script>"); // TODO: log IP
    }
    
    stringstream pathStream;
    pathStream << s_webPath << url;
    
    string path = pathStream.str();
    
    if(!boost::filesystem::exists(path)) return std::make_tuple(404, "404 File Not Found");
    
    std::ifstream stream;
    stream.open(path, std::ios::in);
    
    if(!stream) return std::make_tuple(404, "404 File Not Found (stream null)");
    
    stringstream s;
    s << stream.rdbuf();
    
    return std::make_tuple(200, s.str());
}