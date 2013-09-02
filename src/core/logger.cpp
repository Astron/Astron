#include <time.h>
#include <iostream>
#include <fstream>

#include "logger.h"

Logger::Logger(std::string log_name) : m_output(new std::ofstream(log_name))
{
}

Logger::Logger() : m_output(&std::cout)
{
}

std::ostream &Logger::log(LogSeverity sev)
{
	const char *sevtext;

	switch(sev) {
	case LSEVERITY_SPAM:
		sevtext = "SPAM";
		break;
	case LSEVERITY_DEBUG:
		sevtext = "DEBUG";
		break;
	case LSEVERITY_INFO:
		sevtext = "INFO";
		break;
	case LSEVERITY_WARNING:
		sevtext = "WARNING";
		break;
	case LSEVERITY_SECURITY:
		sevtext = "SECURITY";
		break;
	case LSEVERITY_ERROR:
		sevtext = "ERROR";
		break;
	case LSEVERITY_FATAL:
		sevtext = "FATAL";
		break;
	}

	time_t rawtime;
	time(&rawtime);
	char timetext[1024];
	strftime(timetext, 1024, "%Y-%m-%d %H:%M:%S", localtime(&rawtime));

	*m_output << "[" << timetext << "] " << sevtext << ": ";
	return *m_output;
}

#define LOG_LEVEL(name, severity) \
	std::ostream &Logger::name() \
	{ \
		return log(severity); \
	}

LOG_LEVEL(spam, LSEVERITY_SPAM)
LOG_LEVEL(debug, LSEVERITY_DEBUG)
LOG_LEVEL(info, LSEVERITY_INFO)
LOG_LEVEL(warning, LSEVERITY_WARNING)
LOG_LEVEL(security, LSEVERITY_SECURITY)
LOG_LEVEL(error, LSEVERITY_ERROR)
LOG_LEVEL(fatal, LSEVERITY_FATAL)
