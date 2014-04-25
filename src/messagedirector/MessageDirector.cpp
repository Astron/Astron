#include "MessageDirector.h"
#include "MDNetworkParticipant.h"
#include "core/global.h"
#include "core/msgtypes.h"
#include "config/ConfigVariable.h"
#include "config/constraints.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/icl/interval_bounds.hpp>
using boost::asio::ip::tcp;

static ConfigGroup md_config("messagedirector");
static ConfigVariable<std::string> bind_addr("bind", "unspecified", md_config);
static ConfigVariable<std::string> connect_addr("connect", "unspecified", md_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);
static ValidAddressConstraint valid_connect_addr(connect_addr);

static ConfigGroup daemon_config("daemon");
static ConfigVariable<std::string> daemon_name("name", "<unnamed>", daemon_config);
static ConfigVariable<std::string> daemon_url("url", "", daemon_config);

// Define convenience type
typedef boost::icl::discrete_interval<channel_t> interval_t;

bool ChannelList::qualifies(channel_t channel)
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


MessageDirector::MessageDirector() : m_acceptor(NULL), m_initialized(false), is_client(false),
	m_log("msgdir", "Message Director")
{
}

void MessageDirector::init_network()
{
	if(!m_initialized)
	{
		// Bind to port and listen for downstream servers
		if(bind_addr.get_val() != "unspecified")
		{
			m_log.info() << "Opening listening socket..." << std::endl;
			std::string str_ip = bind_addr.get_val();
			std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
			str_ip = str_ip.substr(0, str_ip.find(':', 0));
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(str_ip, str_port);
			tcp::resolver::iterator it = resolver.resolve(query);
			m_acceptor = new tcp::acceptor(io_service, *it, true);
			start_accept();
		}

		// Connect to upstream server and start handling received messages
		if(connect_addr.get_val() != "unspecified")
		{
			m_log.info() << "Connecting upstream..." << std::endl;
			std::string str_ip = connect_addr.get_val();
			std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
			str_ip = str_ip.substr(0, str_ip.find(':', 0));
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(str_ip, str_port);
			tcp::resolver::iterator it = resolver.resolve(query);
			tcp::socket* remote_md = new tcp::socket(io_service);
			boost::system::error_code ec;
			remote_md->connect(*it, ec);
			if(ec.value() != 0)
			{
				m_log.fatal() << "Could not connect to remote MD at IP: "
				              << connect_addr.get_val() << std::endl;
				m_log.fatal() << "Error code: " << ec.value()
				              << "(" << ec.category().message(ec.value()) << ")"
				              << std::endl;
				exit(1);
			}
			set_socket(remote_md);
			is_client = true;
		}
	}
}

void MessageDirector::route_datagram(MDParticipantInterface *p, DatagramHandle dg)
{
	m_log.trace() << "Processing datagram...." << std::endl;

	std::list<channel_t> channels;
	DatagramIterator dgi(dg);
	std::set<ChannelSubscriber*> receiving_participants;
	try
	{
		uint8_t channel_count = dgi.read_uint8();

		// Route messages to participants
		auto &receive_log = m_log.trace();
		receive_log << "Receivers: ";
		for(uint8_t i = 0; i < channel_count; ++i)
		{
			channel_t channel = dgi.read_channel();
			receive_log << channel << ", ";
			channels.push_back(channel);
		}
		receive_log << std::endl;
	}
	catch(DatagramIteratorEOF &)
	{
		if(p)
		{
			// Log error with receivers output
			m_log.error() << "Detected truncated datagram reading header from '"
			              << p->m_name << "'.\n";
		}
		else
		{
			// Log error with receivers output
			m_log.error() << "Detected truncated datagram reading header from unknown participant.\n";
		}
		return;
	}

	lookup_channels(receiving_participants, channels);

	if(p)
	{
		receiving_participants.erase(p);
	}

	for(auto it = receiving_participants.begin(); it != receiving_participants.end(); ++it)
	{
		auto participant = static_cast<MDParticipantInterface*>(*it);
		DatagramIterator msg_dgi(dg, dgi.tell());
		try
		{
			participant->handle_datagram(dg, msg_dgi);
		}
		catch(DatagramIteratorEOF &)
		{
			// Log error with receivers output
			m_log.error() << "Detected truncated datagram in handle_datagram for '" << participant->m_name << "'"
			              " from participant '" << p->m_name << "'." << std::endl;
			return;
		}
	}

	if(p && is_client)  // Send message upstream
	{
		send_datagram(dg);
		m_log.trace() << "...routing upstream." << std::endl;
	}
	else if(!p) // If there is no participant, then it came from the upstream
	{
		m_log.trace() << "...not routing upstream: It came from there." << std::endl;
	}
	else // Otherwise is root node
	{
		m_log.trace() << "...not routing upstream: There is none." << std::endl;
	}
}

void MessageDirector::on_add_channel(channel_t c)
{
	if(is_client)
	{
		// Send upstream control message
		DatagramPtr dg = Datagram::create(CONTROL_ADD_CHANNEL);
		dg->add_channel(c);
		send_datagram(dg);
	}
}

void MessageDirector::on_remove_channel(channel_t c)
{
	if(is_client)
	{
		// Send upstream control message
		DatagramPtr dg = Datagram::create(CONTROL_REMOVE_CHANNEL);
		dg->add_channel(c);
		send_datagram(dg);
	}
}

void MessageDirector::on_add_range(channel_t lo, channel_t hi)
{
	if(is_client)
	{
		// Send upstream control message
		DatagramPtr dg = Datagram::create(CONTROL_ADD_RANGE);
		dg->add_channel(lo);
		dg->add_channel(hi);
		send_datagram(dg);
	}
}

void MessageDirector::on_remove_range(channel_t lo, channel_t hi)
{
	if(is_client)
	{
		// Send upstream control message
		DatagramPtr dg = Datagram::create(CONTROL_REMOVE_RANGE);
		dg->add_channel(lo);
		dg->add_channel(hi);
		send_datagram(dg);
	}
}

void MessageDirector::start_accept()
{
	tcp::socket *socket = new tcp::socket(io_service);
	tcp::endpoint peerEndpoint;
	m_acceptor->async_accept(*socket, boost::bind(&MessageDirector::handle_accept,
	                         this, socket, boost::asio::placeholders::error));
}

void MessageDirector::handle_accept(tcp::socket *socket, const boost::system::error_code& /*ec*/)
{
	// TODO: We should probably accept the error code and maybe do something about it

	boost::asio::ip::tcp::endpoint remote;
	try
	{
		remote = socket->remote_endpoint();
	}
	catch (std::exception &)
	{
		// A client might disconnect immediately after connecting.
		// If this happens, do nothing. Resolves #122.
		// N.B. due to a Boost.Asio bug, the socket will (may?) still have
		// is_open() == true, so we just catch the exception on remote_endpoint
		// instead.
		start_accept();
		return;
	}
	m_log.info() << "Got an incoming connection from "
	             << remote.address() << ":" << remote.port() << std::endl;
	new MDNetworkParticipant(socket); // It deletes itself when connection is lost
	start_accept();
}

void MessageDirector::add_participant(MDParticipantInterface* p)
{
	m_participants.insert(m_participants.end(), p);
}

void MessageDirector::remove_participant(MDParticipantInterface* p)
{
	unsubscribe_all(p);

	// Stop tracking participant
	m_participants.remove(p);

	// Send out any post-remove messages the participant may have added.
	// N.B. this is done last, because we don't want to send messages
	// through the Director while a participant is being removed, as
	// certain data structures may not have their invariants satisfied
	// during that time.
	p->post_remove();
}

void MessageDirector::receive_datagram(DatagramHandle dg)
{
	route_datagram(NULL, dg);
}

void MessageDirector::receive_disconnect()
{
	m_log.fatal() << "Lost connection to upstream md" << std::endl;
	exit(1);
}
