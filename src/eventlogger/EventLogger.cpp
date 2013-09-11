#include "core/RoleFactory.h"
#include "EventLogger.h"

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7197");

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig), m_log("eventlogger", "Event Logger")
{
	bind(bind_addr.get_rval(roleconfig));
}

void EventLogger::bind(const std::string &addr)
{
	m_log.info() << "Opening UDP socket..." << std::endl;
	std::string str_ip = addr;
	std::string str_port = str_ip.substr(str_ip.find(':', 0)+1, std::string::npos);
	str_ip = str_ip.substr(0, str_ip.find(':', 0));
	udp::resolver resolver(io_service);
	udp::resolver::query query(str_ip, str_port);
	udp::resolver::iterator it = resolver.resolve(query);
	m_socket = new udp::socket(io_service, *it);
}

RoleFactoryItem<EventLogger> el_fact("eventlogger");
