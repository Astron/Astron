#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "util/Datagram.h"

using boost::asio::ip::udp;

class EventSender {
public:
	EventSender();

	void init();
	void send(const Datagram &dg);
private:
	LogCategory m_log;
	udp::socket m_socket;
	udp::endpoint m_target;
	bool m_enabled;
};

extern EventSender g_eventsender;
