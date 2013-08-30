#include <stdio.h>
#include <time.h>

#include "logger.h"

void Logger::log(LogSeverity sev, const char *format, va_list va)
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

	printf("[%s] %s: ", timetext, sevtext);
	vprintf(format, va);
	printf("\n");
}

#define LOG_LEVEL(name, severity) \
	void Logger::name(const char *format, ...) \
	{ \
		va_list args; \
		va_start(args, format); \
		log(severity, format, args); \
		va_end(args); \
	}

LOG_LEVEL(spam, LSEVERITY_SPAM)
LOG_LEVEL(debug, LSEVERITY_DEBUG)
LOG_LEVEL(info, LSEVERITY_INFO)
LOG_LEVEL(warning, LSEVERITY_WARNING)
LOG_LEVEL(security, LSEVERITY_SECURITY)
LOG_LEVEL(error, LSEVERITY_ERROR)
LOG_LEVEL(fatal, LSEVERITY_FATAL)
