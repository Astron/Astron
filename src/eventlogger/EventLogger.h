#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"

using boost::asio::ip::udp;

class EventLogger : public Role
{
public:
	EventLogger(RoleConfig roleconfig);

	void handle_datagram(Datagram &in_dg, DatagramIterator &dgi) { } // Doesn't take DGs.

private:
	LogCategory m_log;
	udp::socket *m_socket;

	void bind(const std::string &addr);
};
