#pragma once
#include <fstream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "util/Datagram.h"

using boost::asio::ip::udp;

// There will typically only be one Event Logger, so we can afford to make the
// receive buffer pretty big.
#define EVENTLOG_BUFSIZE 8192

class EventLogger : public Role
{
public:
	EventLogger(RoleConfig roleconfig);

	void handle_datagram(Datagram &in_dg, DatagramIterator &dgi) { } // Doesn't take DGs.

private:
	LogCategory m_log;
	udp::socket *m_socket;
	udp::endpoint m_remote;
	std::string m_file_format;
	ofstream *m_file;
	char m_buffer[EVENTLOG_BUFSIZE];

	void bind(const std::string &addr);
	void open_log();
	void write_log(const std::vector<std::string> &msg);
	void start_receive();
	void handle_receive(const boost::system::error_code &error, std::size_t bytes);
	void process_packet(const Datagram &dg);
};
