#include "core/global.h"
#include "MessageDirector.h"
#include "core/config.h"
#include "core/global.h"
#include "MDNetworkParticipant.h"
#include <boost/bind.hpp>
using boost::asio::ip::tcp; // I don't want to type all of that god damned shit
ConfigVariable<std::string> bind_addr("messagedirector/bind", "unspecified");
ConfigVariable<std::string> connect_addr("messagedirector/connect", "unspecified");

bool ChannelList::qualifies(unsigned long long channel)
{
	if(is_range)
	{
		//TODO: implement
		return false;
	}
	else
	{
		return channel == a;
	}
}

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
		if(connect_addr.get_val() != "unspecified")
		{
			std::string str_ip = connect_addr.get_val();
			std::string str_port = str_ip.substr(str_ip.find(':', 0)+1, std::string::npos);
			str_ip = str_ip.substr(0, str_ip.find(':', 0));
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(str_ip, str_port);
			tcp::resolver::iterator it = resolver.resolve(query);
			m_remote_md = new tcp::socket(io_service);
			boost::system::error_code ec;
			m_remote_md->connect(*it, ec);
			if(ec.value() != 0)
			{
				gLogger->fatal() << "Could not connect to remote MD at IP: "
					<< connect_addr.get_val() << " With error code: "
					<< ec.value() << "(" << ec.category().message(ec.value()) << ")"
					<< std::endl;
				exit(1);
			}
		}
	}
}

void MessageDirector::handle_datagram(Datagram *dg, MDParticipantInterface *participant)
{
	DatagramIterator dgi(dg);
	unsigned char channels = dgi.read_uint8();
	if(channels == 1)
	{
		unsigned long long channel = dgi.read_uint64();
		if(channel == CONTROL_MESSAGE && participant)
		{
			unsigned int msg_type = dgi.read_uint16();
			switch(msg_type)
			{
				case CONTROL_SET_CHANNEL:
				{
					ChannelList c;
					c.is_range = false;
					c.a = dgi.read_uint64();
					m_participant_channels[participant].insert(m_participant_channels[participant].end(), c);
				}
				break;
				default:
					gLogger->error() << "Unknown MD MsgType: " << msg_type << std::endl;
			}
			return;
		}
		dgi.seek(1);
	}
	for(unsigned char i = 0; i < channels; ++i)
	{
		unsigned long long channel = dgi.read_uint64();
		for(auto it = m_participants.begin(); it != m_participants.end(); ++it)
		{
			MDParticipantInterface *participant = *it;
			for(auto it2 = m_participant_channels[participant].begin(); it2 != m_participant_channels[participant].end(); ++it2)
			{
				if(it2->qualifies(channel))
				{
					participant->handle_datagram(dg, DatagramIterator(dg, 1+(channels*8)));
					break;
				}
			}
		}
	}
}

MessageDirector::MessageDirector() : m_acceptor(NULL), m_initialized(false), m_remote_md(NULL)
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
	boost::asio::ip::tcp::endpoint remote = socket->remote_endpoint();
	gLogger->info() << "Got a Message Director connection from "
	                << remote.address() << ":" << remote.port() << std::endl;
	new MDNetworkParticipant(socket); //It deletes itsself when connection is lost
	start_accept();
}
