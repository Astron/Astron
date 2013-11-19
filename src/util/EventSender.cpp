#include "core/global.h"

#include "EventSender.h"

static ConfigVariable<std::string> target_addr("general/eventlogger", "");

EventSender::EventSender() : m_log("eventsender", "Event Sender"),
	m_socket(io_service, udp::v4()), m_enabled(false)
{

}

void EventSender::init()
{
	std::string str_ip = target_addr.get_val();

	if(str_ip == "")
	{
		m_enabled = false;
		m_log.debug() << "Not enabled." << std::endl;
		return;
	}

	m_log.debug() << "Resolving target..." << std::endl;
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));
	udp::resolver resolver(io_service);
	udp::resolver::query query(str_ip, str_port);
	udp::resolver::iterator it = resolver.resolve(query);
	m_target = *it;
	m_enabled = true;

	m_log.debug() << "Initialized." << std::endl;
}

void EventSender::send(const Datagram &dg)
{
	if(!m_enabled)
	{
		m_log.trace() << "Disabled; discarding event..." << std::endl;
		return;
	}

	m_log.trace() << "Sending event..." << std::endl;
	m_socket.send_to(boost::asio::buffer(dg.get_data(), dg.size()),
	                 m_target);
}

EventSender g_eventsender;
