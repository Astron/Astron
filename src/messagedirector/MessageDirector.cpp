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
		return (channel >= a && channel <= b);
	}
	else
	{
		return channel == a;
	}
}

bool ChannelList::operator==(const ChannelList &rhs)
{
	if(is_range && rhs.is_range)
	{
		return (a == rhs.a && b == rhs.b);
	}
	else if(!is_range && !rhs.is_range)
	{
		return a == rhs.a;
	}
	return false;
}

MessageDirector MessageDirector::singleton;

void MessageDirector::InitializeMD()
{
	if(!m_initialized)
	{
		if(bind_addr.get_val() != "unspecified")
		{
			gLogger->debug() << "binding" << std::endl;
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
			gLogger->debug() << "connecting" << std::endl;
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
			m_buffer = new char[2];
			m_bufsize = 2;
			m_bytes_to_go = 2;
			start_receive();
		}
	}
}

void MessageDirector::handle_datagram(Datagram *dg, MDParticipantInterface *participant)
{
	gLogger->debug() << "MD Handling datagram" << std::endl;
	DatagramIterator dgi(dg);
	unsigned char channels = dgi.read_uint8();
	if(channels == 1)
	{
		unsigned long long channel = dgi.read_uint64();
		if(channel == CONTROL_MESSAGE && participant)
		{
			unsigned int msg_type = dgi.read_uint16();
			bool send_upstream = true;
			switch(msg_type)
			{
				case CONTROL_SET_CHANNEL:
				{
					ChannelList c;
					c.is_range = false;
					c.a = dgi.read_uint64();
					for(auto it = m_participant_channels.begin(); it != m_participant_channels.end(); ++it)
					{
						for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							if(*it2 == c)
							{
								send_upstream = false;
								break;
							}
						}
						if(!send_upstream)
						{
							break;
						}
					}
					m_participant_channels[participant].insert(m_participant_channels[participant].end(), c);
				}
				break;
				case CONTROL_ADD_RANGE:
				{
					ChannelList c;
					c.is_range = true;
					c.a = dgi.read_uint64();
					c.b = dgi.read_uint64();
					for(auto it = m_participant_channels.begin(); it != m_participant_channels.end(); ++it)
					{
						for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							if(*it2 == c)
							{
								send_upstream = false;
								break;
							}
						}
						if(!send_upstream)
						{
							break;
						}
					}
					m_participant_channels[participant].insert(m_participant_channels[participant].end(), c);
				}
				break;
				case CONTROL_REMOVE_CHANNEL:
				{
					ChannelList c;
					c.is_range = false;
					c.a = dgi.read_uint64();
					m_participant_channels[participant].remove(c);
					for(auto it = m_participant_channels.begin(); it != m_participant_channels.end(); ++it)
					{
						for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
						{
							if(*it2 == c)
							{
								send_upstream = false;
								break;
							}
						}
						if(!send_upstream)
						{
							break;
						}
					}
				}
				break;
				default:
					gLogger->error() << "Unknown MD MsgType: " << msg_type << std::endl;
			}
			if(participant && m_remote_md && send_upstream)
			{
				gLogger->debug() << "Sending control message to upstream md" << std::endl;
				unsigned short len = dg->get_buf_end();
				m_remote_md->send(boost::asio::buffer((char*)&len, 2));
				m_remote_md->send(boost::asio::buffer(dg->get_data(), len));
			}
			else if(!send_upstream)
			{
				gLogger->debug() << "not sending upstream because of flag" << std::endl;
			}
			else if(participant)
			{
				gLogger->debug() << "no upstream md for ctrl msg" << std::endl;
			}
			else
			{
				gLogger->debug() << "not redirecting upstream MD's own control message to itsself" << std::endl;
			}
			return;
		}
		dgi.seek(1);
	}
	std::map<MDParticipantInterface*, bool> sent_to_participant;
	for(unsigned char i = 0; i < channels; ++i)
	{
		unsigned long long channel = dgi.read_uint64();
		for(auto it = m_participants.begin(); it != m_participants.end(); ++it)
		{
			if(*it == participant || sent_to_participant[*it])
			{
				continue;
			}
			MDParticipantInterface *participant = *it;
			for(auto it2 = m_participant_channels[participant].begin(); it2 != m_participant_channels[participant].end(); ++it2)
			{
				if(it2->qualifies(channel))
				{
					DatagramIterator msg_dgi(dg, 1+(channels*8));
					bool should_continue = participant->handle_datagram(dg, msg_dgi);
					sent_to_participant[participant] = true;
					if(!should_continue)
					{
						return;
					}
					break;
				}
			}
		}
	}
	
	if(participant && m_remote_md)//if there is no participant, then it came from the upstream
	{
		unsigned short len = dg->get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg->get_data(), len));
		gLogger->debug() << "sending to upstream md" << std::endl;
	}
	else if(!participant)
	{
		gLogger->debug() << "not sending to upstream because it came from upstream" << std::endl;
	}
	else
	{
		gLogger->debug() << "no upstream md" << std::endl;
	}
}

MessageDirector::MessageDirector() : m_acceptor(NULL), m_initialized(false), m_remote_md(NULL),
	m_buffer(NULL), m_bufsize(0), m_bytes_to_go(0), m_is_data(false)
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

void MessageDirector::start_receive()
{
	unsigned short offset = m_bufsize - m_bytes_to_go;
	m_remote_md->async_receive(boost::asio::buffer(m_buffer+offset, m_bufsize-offset), boost::bind(&MessageDirector::read_handler,
		this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void MessageDirector::read_handler(const boost::system::error_code &ec, size_t bytes_transferred)
{
	if(ec.value() != 0)
	{
		gLogger->fatal() << "Lost connection to remote MD error: " << ec.category().message(ec.value()) << std::endl;
		exit(1);
	}
	else
	{
		m_bytes_to_go -= bytes_transferred;
		if(m_bytes_to_go == 0)
		{
			if(!m_is_data)
			{
				m_bufsize = *(unsigned short*)m_buffer;
				delete [] m_buffer;
				m_buffer = new char[m_bufsize];
				m_bytes_to_go = m_bufsize;
				m_is_data = true;
			}
			else
			{
				Datagram dg(std::string(m_buffer, m_bufsize));
				delete [] m_buffer;//dg makes a copy
				m_bufsize = 2;
				m_buffer = new char[m_bufsize];
				m_bytes_to_go = m_bufsize;
				m_is_data = false;
				handle_datagram(&dg, NULL);
			}
		}

		start_receive();
	}
}