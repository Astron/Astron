#include <time.h>
#include <iostream>
#include <fstream>

#include "Logger.h"

NullStream null_stream; // used to print nothing by compiling out the unwanted messages
NullBuffer null_buffer; // used to print nothing by ignoring the unwanted messages

Logger::Logger(const std::string &log_file, LogSeverity sev, bool console_output) :
	m_buf(log_file, console_output), m_severity(sev), m_output(&m_buf)
{	
}

#ifdef ASTRON_DEBUG_MESSAGES
Logger::Logger() : m_buf(), m_severity(LSEVERITY_DEBUG), m_output(&m_buf), m_colorEnable(true)
#else
Logger::Logger() : m_buf(), m_severity(LSEVERITY_INFO), m_output(&m_buf), m_colorEnable(true)
#endif
{
}

static const char* ANSI_GREY = "\x1b[37m";
static const char* ANSI_RED = "\x1b[31;1m";
static const char* ANSI_YELLOW = "\x1b[33;1m";
static const char* ANSI_GREEN = "\x1b[32;1m";
static const char* ANSI_CYAN = "\x1b[36m";
static const char* ANSI_DARK_RED = "\x1b[31";

static const char* ANSI_RESET = "\x1b[0m";

const char* Logger::get_severity_color(LogSeverity sev) {
	switch(sev)
	{
		case LSEVERITY_FATAL:
		case LSEVERITY_ERROR:
			return ANSI_RED;
		case LSEVERITY_SECURITY:
			return ANSI_DARK_RED;
		case LSEVERITY_WARNING:
			return ANSI_YELLOW;
		case LSEVERITY_DEBUG:
		case LSEVERITY_PACKET:
		case LSEVERITY_TRACE:
			return ANSI_CYAN;
		case LSEVERITY_INFO:
			return ANSI_GREEN;
	}
	
	return ANSI_GREY;
}

// log returns an output stream for C++ style stream operations.
LockedLogOutput Logger::log(LogSeverity sev)
{
	const char *sevtext;

	if(sev < m_severity)
	{
		LockedLogOutput null_out(NULL, NULL);
		return null_out;
	}

	switch(sev)
	{
        case LSEVERITY_PACKET:
			sevtext = "PACKET";
			break;
		case LSEVERITY_TRACE:
			sevtext = "TRACE";
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

	LockedLogOutput out(&m_output, &m_lock);
	
	out << "[" << timetext << "] ";
		
	if(m_colorEnable) out << get_severity_color(sev);
	
	out << sevtext;
	
	if(m_colorEnable) out << ANSI_RESET;
	out << ": ";
	return out;
}

// set_min_serverity sets the lowest severity that will be output to the log.
// Messages with lower severity levels will be discarded.
void Logger::set_min_severity(LogSeverity sev)
{
	m_severity = sev;
}

// get_min_severity returns the current minimum severity that will be logged by the logger.
LogSeverity Logger::get_min_severity()
{
	return m_severity;
}

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
