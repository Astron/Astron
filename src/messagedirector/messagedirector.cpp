#include "messagedirector.h"
#include "../core/config.h"
ConfigVariable<std::string> bind_addr("messagedirector/bind", "unspecified");

MessageDirector MessageDirector::singleton;

void MessageDirector::InitializeMD()
{
	if(!m_initialized)
	{
		if(bind_addr.get_val() != "unspecified")
		{
		}
	}
}

MessageDirector::MessageDirector() : m_acceptor(NULL), m_initialized(false)
{
}