#include <string>

#include <ctime>
#include <cctype>
#include <cstdio>

#include "core/RoleFactory.h"
#include "config/constraints.h"
#include "EventLogger.h"
#include "util/EventSender.h"

#include "msgpack_decode.h"

static RoleConfigGroup el_config("eventlogger");
static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7197", el_config);
static ConfigVariable<std::string> output_format("output", "events-%Y%m%d-%H%M%S.csv", el_config);
static ConfigVariable<std::string> rotate_interval("rotate_interval", "0", el_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig),
	m_log("eventlogger", "Event Logger"), m_file(0)
{
	bind(bind_addr.get_rval(roleconfig));

	m_file_format = output_format.get_rval(roleconfig);
	if(m_file_format[0] == '~') {
		open_pipe_log();
	} else {
		open_log();
	}

	LoggedEvent event("log-opened", "EventLogger");
	event.add("msg", "Log opened upon Event Logger startup.");
	process_packet(event.make_datagram());

	start_receive();
}

void EventLogger::bind(const std::string &addr)
{
	m_log.info() << "Opening UDP socket..." << std::endl;
	std::string str_ip = addr;
	std::string str_port = str_ip.substr(str_ip.find(':', 0) + 1, std::string::npos);
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
	{
		m_file->close();
	}

	m_file = new std::ofstream(filename);

	m_log.info() << "Opened new log." << std::endl;
}

void EventLogger::open_pipe_log()
{
	m_pipe = popen(m_file_format.substr(1).c_str(), "w");
}

void EventLogger::cycle_log()
{
    	if(m_pipe) {
        	m_log.trace() << "Pipes cannot have cycled logs" << std::endl;
      		return;
    	}
   	 open_log();

	LoggedEvent event("log-opened", "EventLogger");
	event.add("msg", "Log cycled.");
	process_packet(event.make_datagram());
}

void EventLogger::process_packet(const DatagramHandle dg)
{
	DatagramIterator dgi(dg);
	std::stringstream stream;

	try
	{
		msgpack_decode(stream, dgi);
	}
	catch(DatagramIteratorEOF&)
	{
		m_log.error() << "Received truncated packet from "
						<< m_remote.address() << ":" << m_remote.port() << std::endl;
		return;
	}

	if(dgi.tell() != dg->size())
	{
		m_log.error() << "Received packet with extraneous data from "
						<< m_remote.address() << ":" << m_remote.port() << std::endl;
		return;
	}

	std::string data = stream.str();
	m_log.trace() << "Received: " << data << std::endl;

	// This is a little bit of a kludge, but we should make sure we got a
	// MessagePack map as the event log element, and not some other type. The
	// easiest way to do this is to make sure that the JSON representation
	// begins with {
	if(data[0] != '{')
	{
		m_log.error() << "Received non-map event log from "
						<< m_remote.address() << ":" << m_remote.port()
						<< ": " << data << std::endl;
		return;
	}

	// Now let's insert our timestamp:
	time_t rawtime;
	time(&rawtime);

	char timestamp[64];
	strftime(timestamp, 64, "{\"_time\": \"%Y-%m-%d %H:%M:%S%z\", ", localtime(&rawtime));

	if(m_pipe) {
	   std::stringstream ss;
       ss << timestamp << data.substr(1) << std::endl;
    
       fprintf(m_pipe, "%s", ss.str().c_str());
       fflush(m_pipe);
	} else {
	   *m_file << timestamp << data.substr(1) << std::endl;
    }
	
}

void EventLogger::start_receive()
{
	m_socket->async_receive_from(boost::asio::buffer(m_buffer, EVENTLOG_BUFSIZE),
	                             m_remote, boost::bind(&EventLogger::handle_receive, this,
	                                     boost::asio::placeholders::error,
	                                     boost::asio::placeholders::bytes_transferred));
}

void EventLogger::handle_receive(const boost::system::error_code &ec, std::size_t bytes)
{
	if(ec.value())
	{
		m_log.warning() << "While receiving packet from "
		                << m_remote.address() << ":" << m_remote.port()
		                << ", an error occurred: "
		                << ec.value() << std::endl;
		return;
	}

	m_log.trace() << "Got packet from "
	              << m_remote.address() << ":" << m_remote.port() << std::endl;

	DatagramPtr dg = Datagram::create(m_buffer, bytes);
	process_packet(dg);

	start_receive();
}

static RoleFactoryItem<EventLogger> el_fact("eventlogger");
