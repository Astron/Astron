#include <ctime>

#include "core/RoleFactory.h"
#include "EventLogger.h"

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7197");
static ConfigVariable<std::string> output_format("output", "events-%Y%m%d-%H%M%S.csv");

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig), m_log("eventlogger", "Event Logger")
{
	bind(bind_addr.get_rval(roleconfig));

	m_file_format = output_format.get_rval(roleconfig);
	open_log();
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

void EventLogger::open_log()
{
	time_t rawtime;
	time(&rawtime);

	char filename[1024];
	strftime(filename, 1024, m_file_format.c_str(), localtime(&rawtime));
	m_log.debug() << "New log filename: " << filename << std::endl;

	if(m_file)
		m_file->close();

	m_file = new ofstream(filename);

	m_log.info() << "Opened new log." << std::endl;
}

static RoleFactoryItem<EventLogger> el_fact("eventlogger");
