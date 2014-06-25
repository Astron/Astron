#include "HTTPServer.h"
#include "frozenweb.h"

using boost::asio::ip::tcp;
using namespace std;

static string s_webPath;

static LogCategory weblog("web", "Web");
 
HTTPServer::HTTPServer(string str_ip, string str_port, string web_path)
{    
    s_webPath = web_path;
    
    if(s_webPath == "FROZEN") initFrozen();
    
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
	m_ipAddress = m_socket.remote_endpoint().address().to_string();
	
    m_socket.async_read_some( boost::asio::buffer(m_request, MAX_HTTP_DATA_LENGTH), 
                               boost::bind(&HTTPConnection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
}

void HTTPConnection::handle_read(const boost::system::error_code& /* ec */,
                    size_t /* bytes_transferred*/)
{
    // split request into headers, method, and data (see HTTP protocol)
	
    std::vector<std::string> headers;
    boost::split(headers, m_request, boost::is_any_of("\r\n"));
    
    if(headers.size() < 2) return;
    
    std::vector<std::string> topLine;
    boost::split(topLine, headers[0], boost::is_any_of(" "));
    
    if(topLine.size() < 3) return;
    
    string requestType = topLine[0];
    string url = topLine[1];
    
	// decode URL
	
    std::vector<std::string> urlVsData;
    boost::split(urlVsData, url, boost::is_any_of("?"));
        
    std::vector<std::string> urlParts;
    boost::split(urlParts, urlVsData[0], boost::is_any_of("/"));
    
    std::vector<std::string> dataParts;
    
    if(urlVsData.size() > 1)
    {
        boost::split(dataParts, urlVsData[1], boost::is_any_of("&"));
    }
	
	//serve request depending on type
    
    errorMimeResponse_t resp = HTTPServer::serve404("Malformed request");
        
    if(urlParts.size() < 2) {
        resp = HTTPServer::serve404();
    } else if(urlParts[1] == "admin") {
        resp = HTTPServer::handleAppPage(url, m_ipAddress);
    } else if(urlParts[1] == "request" && urlVsData.size() > 1) {
        // create a map from URL parameters of form ?a=b&c=d&e=f 
        
        map <string, string> parameters;
        
        for(int i = 0; i < (int) dataParts.size(); ++i)
        {
            std::vector<std::string> dparts;
            boost::split(dparts, dataParts[i], boost::is_any_of("="));
            parameters[dparts[0]] = dparts[1];
        }
        
        resp = HTTPServer::handleRequest(urlParts[2], parameters, requestType, m_ipAddress);
    }
	
	// serve HTTP response
    
    stringstream s;
    
    s   << "HTTP/1.1 " << std::get<0>(resp) << " OK\n"
        << "Server: Astron Web Administration\n"
        << "Content-Type: " << std::get<1>(resp) << "\n"
        << "Content-Length: " << std::get<2>(resp).length() << "\n"
        << "Connection: close\n\n" 
            
        << std::get<2>(resp) << "\n";
    
    boost::asio::async_write(m_socket, boost::asio::buffer(s.str()), 
        boost::bind(&HTTPConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

// requestName is stated in the URL
// params is in URL GET request form
// method is the HTTP method, GET, POST, etc.

errorMimeResponse_t HTTPServer::handleRequest(	std::string /*requestName*/, 
												std::map <std::string, std::string> /*params*/, 
												std::string /*method*/, 
												std::string /*ip*/)
{                                                             
    return serveRequest(200, "{\"success\" : \"1\"}");
}

// HTTPServer::handleAppPage
// called when the request is in the application space
// currently /admin/

errorMimeResponse_t HTTPServer::handleAppPage(std::string url, std::string ip)
{   
	// determine mime type, defaults to text/plain
    string mimeType = "text/plain";
        
    std::vector<std::string> fileParts;
    boost::split(fileParts, url, boost::is_any_of(".")) ;
    
    if(fileParts.size() > 1)
    {
        mimeType = mimeFromExt(fileParts[1]);
    }
    
	// configurable frozen mode: skip filesystem access for security, portabillity, and performance
    if(s_webPath == "FROZEN")
    {
        return serveFrozenPage(url, mimeType);
    }
    
	// protect against directory traversal attack
    if(detectDirectoryTraversal(url))
    {
		weblog.security() << "Directory Traversal Attack Detected from " << ip << " at " << url << "\n";
        return serveFile("text/html", "<script>alert('Stop hacking');</script>"); // TODO: log IP
    }
	
	// determine relative path
	
    stringstream pathStream;
    pathStream << s_webPath << url;
    
    string path = pathStream.str();
    
	// 404 if non-existent
	
    if(!boost::filesystem::exists(path))
    {
        return serve404();
    } 
    
	// read file and serve to browser
	
    std::ifstream stream;
    stream.open(path, std::ios::in);
    
    if(!stream)
    {
        return serve404("Stream null");
    } 
    
    stringstream s;
    s << stream.rdbuf();
    
    return serveFile(mimeType, s.str());
}

errorMimeResponse_t HTTPServer::serveFrozenPage(std::string url, std::string mimeType)
{    
	// ensure frozen file exists
	
    if(!g_frozenWeb.count(url))
    {
        return serve404();
    }
    
    return serveFile(mimeType, g_frozenWeb[url]);
}

errorMimeResponse_t HTTPServer::serve404(std::string info)
{
    stringstream message;
    
    message << "<h1>404 File Not Found</h1>";
    
	//info defaults to length 0, optional additional info support
	
    if(info.length()) {
        message << "<h2>Additional Info</h2>" << info;
    }
    
    return std::make_tuple(404, "text/html", message.str());
}

errorMimeResponse_t HTTPServer::serveRequest(int error, std::string request)
{
    return std::make_tuple(error, "text/plain", request);
}

errorMimeResponse_t HTTPServer::serveFile(std::string mimeType, std::string content)
{
    return std::make_tuple(200, mimeType, content);
}

std::string HTTPServer::mimeFromExt(std::string ext)
{
    if(ext == "html") return "text/html";
    if(ext == "css") return "text/css";
    
    if(ext == "js") return "application/x-javascript";
    
    if(ext == "bmp") return "image/bmp";
    if(ext == "jpg") return "image/jpeg";
    if(ext == "png") return "image/png";
    
    if(ext == "wav") return "audio/x-wav";
    if(ext == "mp3") return "audio/mpeg";
    
    return "text/plain";   
}

bool HTTPServer::detectDirectoryTraversal(std::string url) 
{
    return	boost::find_first(url, "..")
		 || boost::find_first(url, "~")
		 || boost::find_first(url, "%2e%2e");
}