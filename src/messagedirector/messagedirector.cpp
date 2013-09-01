#include "../core/global.h"
#include "messagedirector.h"
#include "../core/config.h"
#include "MDNetworkParticipant.h"
#include <boost/bind.hpp>
using boost::asio::ip::tcp; // I don't want to type all of that god damned shit
ConfigVariable<std::string> bind_addr("messagedirector/bind", "unspecified");

MessageDirector MessageDirector::singleton;

void MessageDirector::InitializeMD()
{
	if(!m_initialized)
	{
		if(bind_addr.get_val() != "unspecified")
		{
			std::string str_ip = bind_addr.get_val();
			std::string str_port = str_ip.substr(str_ip.find(':', 0)+1, std::string::npos);
			str_ip = str_ip.substr(0, str_ip.find(':', 0));
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(str_ip, str_port);
			tcp::resolver::iterator it = resolver.resolve(query);
			m_acceptor = new tcp::acceptor(io_service, *it, true);
			start_accept();
		}
	}
}

MessageDirector::MessageDirector() : m_acceptor(NULL), m_initialized(false)
{
}

void MessageDirector::start_accept()
{
	tcp::socket *socket = new tcp::socket(io_service);
	tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(*socket, boost::bind(&MessageDirector::handle_accept, 
		this, socket, boost::asio::placeholders::error));
}

void MessageDirector::handle_accept(tcp::socket *socket, const boost::system::error_code &ec)
{
	gLogger->info("Got a client connection");
	new MDNetworkParticipant(socket); //It deletes itsself when connection is lost
	start_accept();
}