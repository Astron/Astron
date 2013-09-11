#include "core/RoleFactory.h"
#include "EventLogger.h"

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig), m_log("eventlogger", "Event Logger")
{

}

RoleFactoryItem<EventLogger> el_fact("eventlogger");
