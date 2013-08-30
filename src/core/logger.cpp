#include <stdio.h>
#include <time.h>

#include "logger.h"

void Logger::log(LogSeverity sev, const char *format, va_list va)
{
	const char *sevtext;

	switch(sev) {
	case SEVERITY_SPAM:
		sevtext = "SPAM";
		break;
	case SEVERITY_DEBUG:
		sevtext = "DEBUG";
		break;
	case SEVERITY_INFO:
		sevtext = "INFO";
		break;
	case SEVERITY_WARNING:
		sevtext = "WARNING";
		break;
	case SEVERITY_SECURITY:
		sevtext = "SECURITY";
		break;
	case SEVERITY_ERROR:
		sevtext = "ERROR";
		break;
	case SEVERITY_FATAL:
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

LOG_LEVEL(spam, SEVERITY_SPAM)
LOG_LEVEL(debug, SEVERITY_DEBUG)
LOG_LEVEL(info, SEVERITY_INFO)
LOG_LEVEL(warning, SEVERITY_WARNING)
LOG_LEVEL(security, SEVERITY_SECURITY)
LOG_LEVEL(error, SEVERITY_ERROR)
LOG_LEVEL(fatal, SEVERITY_FATAL)
