#include "ClientFactory.h"
static LogCategory cf_log("clientfactory", "clientfactory");

ClientFactory::ClientFactory() : m_client_type(NULL)
{
}

ClientFactory& ClientFactory::get_singleton()
{
	static ClientFactory cf;
	return cf;
}

void ClientFactory::set_client_type(BaseClientType *client_type)
{
	if(!m_client_type || client_type->priority() > m_client_type->priority())
	{
		m_client_type = client_type;
	}
}

Client* ClientFactory::create(boost::asio::ip::tcp::socket *socket, LogCategory *log,
	RoleConfig roleconfig, ChannelTracker *ct)
{
	if(!m_client_type)
	{
		cf_log.fatal() << "m_client_type was never set.";
		exit(1);
	}
	return m_client_type->instantiate(socket, log, roleconfig, ct);
}

BaseClientType::BaseClientType(unsigned char priority) : m_priority(priority)
{
	ClientFactory::get_singleton().set_client_type(this);
}

unsigned char BaseClientType::priority()
{
	return m_priority;
}