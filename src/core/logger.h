#pragma once
#include <stdarg.h>
#include <string>

class LoggerBuf : public std::streambuf
{
	public:
		LoggerBuf();
		LoggerBuf(const std::string &file_name, bool output_to_console = true);
	protected:
		int overflow(int c = EOF);
		std::streamsize xsputn (const char* s, std::streamsize n);
	private:
		bool m_output_to_console;
		bool m_has_file;
		std::ofstream m_file;
};

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
	Logger(const std::string &log_file, bool console_output = true);
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
	LoggerBuf m_buf;
	std::ostream m_output;
};
