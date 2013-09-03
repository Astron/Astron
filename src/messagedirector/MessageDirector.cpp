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
		// Bind to port and listen for downstream servers
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

		// Connect to upstream server and start handling recieved messages
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

	// Check for control messages
	if(channels == 1)
	{
		unsigned long long channel = dgi.read_uint64();

		if(channel == CONTROL_MESSAGE && participant)
		{
			unsigned int msg_type = dgi.read_uint16();
			bool send_upstream = true;
			switch(msg_type)
			{
				case CONTROL_ADD_CHANNEL:
				{
					send_upstream = false;//handled by function
					subscribe_channel(participant, dgi.read_uint64());
				}
				break;
				case CONTROL_ADD_RANGE:
				{
					send_upstream = false;//handled by function
					unsigned long long lo = dgi.read_uint64();
					unsigned long long hi = dgi.read_uint64();
					subscribe_range(participant, lo, hi);
				}
				break;
				case CONTROL_REMOVE_CHANNEL:
				{
					send_upstream = false;//handled by function
					unsubscribe_channel(participant, dgi.read_uint64());
				}
				break;
				case CONTROL_REMOVE_RANGE:
				{
					send_upstream = false;//handled by function
					unsigned long long lo = dgi.read_uint64();
					unsigned long long hi = dgi.read_uint64();
					unsubscribe_range(participant, lo, hi);
				}
				break;
				case CONTROL_ADD_POST_REMOVE:
				{
					send_upstream = false;
					std::string data = dgi.read_string();
					participant->set_post_remove(data);
				}
				break;
				case CONTROL_CLEAR_POST_REMOVE:
				{
					send_upstream = false;
					participant->set_post_remove("");
				}
				break;
				default:
					gLogger->error() << "Unknown MD MsgType: " << msg_type << std::endl;
			}

			// TODO: Since send_upstream logic for many control actions was moved, check how much of this is necessary
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
				gLogger->debug() << "not redirecting upstream MD's own control message to itself" << std::endl;
			}
			return;
		}

		// return offset to beginning of recipients if not a CONTROL_MESSAGE
		dgi.seek(1);
	}

	// Route messages to participants
	std::set<MDParticipantInterface*> receiving_participants;
	for(unsigned char i = 0; i < channels; ++i)
	{
		unsigned long long channel = dgi.read_uint64();
		for(auto it = m_channel_subscriptions[channel].begin(); it != m_channel_subscriptions[channel].end(); ++it)
		{
			receiving_participants.insert(receiving_participants.end(), *it);
		}
		std::set<MDParticipantInterface*> range = boost::icl::find(m_range_subscriptions, channel)->second;
		receiving_participants.insert(range.begin(), range.end());
	}

	if (participant)
	{
		receiving_participants.erase(participant);
	}

	for(auto it = receiving_participants.begin(); it != receiving_participants.end(); ++it)
	{
		DatagramIterator msg_dgi(dg, 1+channels*8);
		(*it)->handle_datagram(dg, msg_dgi);
	}

	if(participant && m_remote_md)  // Send message upstream
	{
		unsigned short len = dg->get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg->get_data(), len));
		gLogger->debug() << "sending to upstream md" << std::endl;
	}
	else if(!participant) // If there is no participant, then it came from the upstream
	{
		gLogger->debug() << "not sending to upstream because it came from upstream" << std::endl;
	}
	else // Otherwise is root node
	{
		gLogger->debug() << "no upstream md" << std::endl;
	}
}

void MessageDirector::subscribe_channel(MDParticipantInterface* p, unsigned long long a)
{
	ChannelList c;
	c.is_range = false;
	c.a = a;

	gLogger->debug() << "Subscribe A-" << a << endl;

	bool should_upstream = should_add_upstream(c);

	std::set<MDParticipantInterface*> range = boost::icl::find(m_range_subscriptions, a)->second;
	if (range.find(p) == range.end()) {
		p->channels().insert(p->channels().end(), c);
		m_channel_subscriptions[a].insert(m_channel_subscriptions[a].end(), p);
	}

	if(should_upstream)
	{
		gLogger->debug() << "Sent upstream - " << a << endl;
		Datagram dg;
		dg.add_uint8(1);
		dg.add_uint64(CONTROL_MESSAGE);
		dg.add_uint16(CONTROL_ADD_CHANNEL);
		dg.add_uint64(a);
		unsigned short len = dg.get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg.get_data(), len));
	}
}

void MessageDirector::unsubscribe_channel(MDParticipantInterface* p, unsigned long long a)
{
	ChannelList c;
	c.is_range = false;
	c.a = a;

	p->channels().remove(c);
	m_channel_subscriptions[a].erase(p);

	if(should_remove_upstream(c))
	{
		Datagram dg(CONTROL_REMOVE_CHANNEL);
		dg.add_uint64(a);
		unsigned short len = dg.get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg.get_data(), len));
	}
}

void MessageDirector::subscribe_range(MDParticipantInterface* p, unsigned long long a, unsigned long long b)
{
	std::set<MDParticipantInterface*> participant_set;
	participant_set.insert(p);

	ChannelList c;
	c.is_range = true;
	c.a = a;
	c.b = b;

	bool should_upstream = should_add_upstream(c);

	p->channels().insert(p->channels().end(), c);
	m_range_subscriptions += std::make_pair(
								boost::icl::discrete_interval<unsigned long long>::closed(a, b),
								participant_set);

	std::list<ChannelList> channels = p->channels();
	std::list<ChannelList>::iterator it = channels.begin();
	while (it != channels.end())
	{
		std::list<ChannelList>::iterator prev = it++;

		if (!it->is_range && it->a >= a && it->a <= b) {
			m_channel_subscriptions[a].erase(p);
			p->channels().erase(prev);
		}
	}

	if(should_upstream)
	{
		Datagram dg(CONTROL_ADD_RANGE);
		dg.add_uint64(a);
		dg.add_uint64(b);
		unsigned short len = dg.get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg.get_data(), len));
	}
}

void MessageDirector::unsubscribe_range(MDParticipantInterface *p, unsigned long long a, unsigned long long b)
{
	std::set<MDParticipantInterface*> participant_set;
	participant_set.insert(p);

	ChannelList c;
	c.is_range = true;
	c.a = a;
	c.b = b;

	p->channels().remove(c);
	m_range_subscriptions -= std::make_pair(
								boost::icl::discrete_interval<unsigned long long>::closed(a, b),
								participant_set);

	if(should_remove_upstream(c))
	{
		Datagram dg;
		dg.add_uint8(1);
		dg.add_uint64(CONTROL_MESSAGE);
		dg.add_uint16(CONTROL_REMOVE_RANGE);
		dg.add_uint64(a);
		dg.add_uint64(b);
		unsigned short len = dg.get_buf_end();
		m_remote_md->send(boost::asio::buffer((char*)&len, 2));
		m_remote_md->send(boost::asio::buffer(dg.get_data(), len));
	}

}

// should_add_upstream should be called before make processing control messages internally
inline bool MessageDirector::should_add_upstream(ChannelList c)
{
	// Don't route upstream if a previous subscription exists, already routed
	if(m_channel_subscriptions[c.a].size() > 0) {
		return false;
	}

	// Don't route upstream if a previous subscription exists, already routed
	if(c.is_range) {
		auto interval_range = m_range_subscriptions.equal_range(boost::icl::discrete_interval<unsigned long long>::closed(c.a, c.b));
		for(auto it = interval_range.first; it != interval_range.second; ++it)
		{
			if (it->second.size() > 0) {
				return false;
			}
		}
	} else {
		auto participants = boost::icl::find(m_range_subscriptions, c.a)->second;
		if(!participants.empty()) {
			return false;
		}
	}

	// Return true if not root
	return m_remote_md;
}

// should_remove_upstream should be called after processing control messages internally
inline bool MessageDirector::should_remove_upstream(ChannelList c)
{
	// Don't route upstream if any more subscriptions exist
	if(m_channel_subscriptions[c.a].size() > 0) {
		return false;
	}

	// Don't route upstream if any more subscriptions exist
	if(c.is_range) {
		auto interval_range = m_range_subscriptions.equal_range(boost::icl::discrete_interval<unsigned long long>::closed(c.a, c.b));
		for(auto it = interval_range.first; it != interval_range.second; ++it)
		{
			if (it->second.size() > 0) {
				return false;
			}
		}
	} else {
		auto participants = boost::icl::find(m_range_subscriptions, c.a)->second;
		if(!participants.empty()) {
			return false;
		}
	}

	// Return true if not root
	return m_remote_md;
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

				gLogger->debug() << "Reader handle datagram" << endl;
				handle_datagram(&dg, NULL);
			}
		}

		start_receive();
	}
}

void MessageDirector::add_participant(MDParticipantInterface* participant)
{
	m_participants.insert(m_participants.end(), participant);
}

void MessageDirector::remove_participant(MDParticipantInterface* participant)
{
	if(participant->post_remove().length() > 0)
	{
		Datagram dg(participant->post_remove());
		handle_datagram(&dg, participant);
	}
	std::list<ChannelList> channels = participant->channels();
	for(auto it = channels.begin(); it != channels.end();)
	{
		Datagram dg;
		dg.add_uint8(1);
		dg.add_uint64(CONTROL_MESSAGE);
		if(it->is_range)
		{
			std::set<MDParticipantInterface*> participant_set;
			participant_set.insert(participant);

			m_range_subscriptions -= std::make_pair(
				boost::icl::discrete_interval<unsigned long long>::closed(it->a, it->b), participant_set);

			dg.add_uint16(CONTROL_REMOVE_RANGE);
			dg.add_uint64(it->a);
			dg.add_uint64(it->b);
		}
		else
		{
			m_channel_subscriptions[it->a].erase(participant);

			dg.add_uint16(CONTROL_REMOVE_CHANNEL);
			dg.add_uint64(it->a);
		}

		handle_datagram(&dg, participant);
		it++;
	}
	m_participants.remove(participant);
}
