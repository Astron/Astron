#include <ctime>
#include <cctype>

#include "core/RoleFactory.h"
#include "EventLogger.h"

static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7197");
static ConfigVariable<std::string> output_format("output", "events-%Y%m%d-%H%M%S.csv");

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig), m_log("eventlogger", "Event Logger")
{
	bind(bind_addr.get_rval(roleconfig));

	m_file_format = output_format.get_rval(roleconfig);
	open_log();
	std::vector<std::string> msg;
	msg.push_back("EventLogger");
	msg.push_back("log_opened");
	msg.push_back("Log opened upon Event Logger startup.");
	write_log(msg);

	start_receive();
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

void EventLogger::cycle_log()
{
	open_log();

	std::vector<std::string> msg;
	msg.push_back("EventLogger");
	msg.push_back("log_opened");
	msg.push_back("Log cycled.");
	write_log(msg);
}

void EventLogger::write_log(const std::vector<std::string> &msg)
{
	time_t rawtime;
	time(&rawtime);

	char timestamp[1024];
	strftime(timestamp, 1024, "\"%Y-%m-%d %H:%M:%S\"", localtime(&rawtime));

	*m_file << timestamp;

	for(auto it = msg.begin(); it != msg.end(); ++it)
	{
		// If the column consists purely of alphanumerics, we can just dump it
		// without quoting it...
		bool alnum = true;
		for(const char *c = it->c_str(); *c; ++c)
		{
			if(!isalnum(*c) && *c != '_')
			{
				alnum = false;
				break;
			}
		}
		if(alnum)
		{
			*m_file << ',' << *it;
			continue;
		}

		// Okay, there's other stuff in this column, so we're going to quote:

		char *cleaned = new char[it->length()*2+1];

		char *p = cleaned;
		for(const char *c = it->c_str(); *c; ++c)
		{
			if(*c == '"')
			{
				*(p++) = '"';
				*(p++) = '"';
			}
			else
			{
				*(p++) = *c;
			}
		}
		*p = 0;

		*m_file << ",\"" << cleaned << '"';
		delete [] cleaned;
	}

	*m_file << "\r\n" << std::flush;
}

void EventLogger::process_packet(const Datagram &dg)
{
	DatagramIterator dgi(dg);

	std::vector<std::string> msg;

	while(dgi.tell() != dg.size())
	{
		try
		{
			msg.push_back(dgi.read_string());
		}
		catch(std::exception &e)
		{
			m_log.error() << "Received truncated packet from "
			              << m_remote.address() << ":" << m_remote.port() << std::endl;
			return;
		}
	}

	write_log(msg);
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

	m_log.spam() << "Got packet from "
	             << m_remote.address() << ":" << m_remote.port() << std::endl;

	Datagram dg(m_buffer, bytes);
	process_packet(dg);

	start_receive();
}

static RoleFactoryItem<EventLogger> el_fact("eventlogger");
