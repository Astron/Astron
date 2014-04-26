#include "MDNetworkUpstream.h"
#include "MessageDirector.h"
#include "core/global.h"
#include "core/msgtypes.h"

using boost::asio::ip::tcp;

MDNetworkUpstream::MDNetworkUpstream(MessageDirector *md) :
	m_message_director(md)
{

}

boost::system::error_code MDNetworkUpstream::connect(const std::string &address)
{
	std::string str_ip = address;
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
		delete remote_md;
		return ec;
	}

	set_socket(remote_md);

	return ec;
}

void MDNetworkUpstream::subscribe_channel(channel_t c)
{
	DatagramPtr dg = Datagram::create(CONTROL_ADD_CHANNEL);
	dg->add_channel(c);
	send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_channel(channel_t c)
{
	DatagramPtr dg = Datagram::create(CONTROL_REMOVE_CHANNEL);
	dg->add_channel(c);
	send_datagram(dg);
}

void MDNetworkUpstream::subscribe_range(channel_t lo, channel_t hi)
{
	DatagramPtr dg = Datagram::create(CONTROL_ADD_RANGE);
	dg->add_channel(lo);
	dg->add_channel(hi);
	send_datagram(dg);
}

void MDNetworkUpstream::unsubscribe_range(channel_t lo, channel_t hi)
{
	DatagramPtr dg = Datagram::create(CONTROL_REMOVE_RANGE);
	dg->add_channel(lo);
	dg->add_channel(hi);
	send_datagram(dg);
}

void MDNetworkUpstream::handle_datagram(DatagramHandle dg)
{
	send_datagram(dg);
}

void MDNetworkUpstream::receive_datagram(DatagramHandle dg)
{
	m_message_director->receive_datagram(dg);
}

void MDNetworkUpstream::receive_disconnect()
{
	m_message_director->receive_disconnect();
}
