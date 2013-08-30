#pragma once
#include <stdarg.h>

enum LogSeverity {
	SEVERITY_SPAM,
	SEVERITY_DEBUG,
	SEVERITY_INFO,
	SEVERITY_WARNING,
	SEVERITY_SECURITY,
	SEVERITY_ERROR,
	SEVERITY_FATAL
};

class Logger {
public:
	void log(LogSeverity sev, const char *format, va_list va);
	void spam(const char *format, ...);
	void debug(const char *format, ...);
	void info(const char *format, ...);
	void warning(const char *format, ...);
	void security(const char *format, ...);
	void error(const char *format, ...);
	void fatal(const char *format, ...);
};
