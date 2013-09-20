#include <time.h>
#include <iostream>
#include <fstream>

#include "logger.h"

NullStream null_stream;

LoggerBuf::LoggerBuf() : std::streambuf(), m_has_file(false), m_output_to_console(true)
{
}

LoggerBuf::LoggerBuf(const std::string &file_name, bool output_to_console) :
	m_file(file_name), m_has_file(true), m_output_to_console(output_to_console)
{
	if(!m_file.is_open())
	{
		m_has_file = false;
	}
}

int LoggerBuf::overflow(int c)
{
	if(m_output_to_console)
	{
		std::cout.put(c);
	}
	if(m_has_file)
	{
		m_file.put(c);
	}
	return c;
}

std::streamsize LoggerBuf::xsputn(const char* s, std::streamsize n)
{
	if(m_output_to_console)
	{
		std::cout.write(s, n);
	}
	if(m_has_file)
	{
		m_file.write(s, n);
	}
	return n;
}

Logger::Logger(const std::string &log_file, bool console_output) : m_buf(log_file, console_output),
	m_output(&m_buf)
{
}

Logger::Logger() : m_buf(), m_output(&m_buf)
{
}

std::ostream &Logger::log(LogSeverity sev)
{
	const char *sevtext;

	switch(sev)
	{
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

	m_output << "[" << timetext << "] " << sevtext << ": ";
	return m_output;
}
