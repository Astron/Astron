#pragma once
#include <stdarg.h>
#include <string>

enum LogSeverity {
	LSEVERITY_SPAM,
	LSEVERITY_DEBUG,
	LSEVERITY_INFO,
	LSEVERITY_WARNING,
	LSEVERITY_SECURITY,
	LSEVERITY_ERROR,
	LSEVERITY_FATAL
};

class Logger {
public:
	Logger(std::string log_file);
	Logger();
	std::ostream &log(LogSeverity sev);
	std::ostream &spam();
	std::ostream &debug();
	std::ostream &info();
	std::ostream &warning();
	std::ostream &security();
	std::ostream &error();
	std::ostream &fatal();
private:
	std::ostream *m_output;
};
