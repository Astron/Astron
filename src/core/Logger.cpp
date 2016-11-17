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
Logger::Logger() : m_buf(), m_severity(LSEVERITY_DEBUG), m_output(&m_buf), m_color_enabled(true)
#else
Logger::Logger() : m_buf(), m_severity(LSEVERITY_INFO), m_output(&m_buf), m_color_enabled(true)
#endif
{
}

/* Reset code */
static const char* ANSI_RESET = "\x1b[0m";

/* Normal Colors */
static const char* ANSI_RED = "\x1b[31m";
static const char* ANSI_GREEN = "\x1b[32m";
static const char* ANSI_ORANGE = "\x1b[33m";
static const char* ANSI_YELLOW = "\x1b[33;2m";
//static const char* ANSI_BLUE = "\x1b[34m";
//static const char* ANSI_CYAN = "\x1b[36m";
static const char* ANSI_GREY = "\x1b[37m";

/* Bold Colors */
//static const char* ANSI_BOLD_RED = "\x1b[31;1m";
//static const char* ANSI_BOLD_GREEN = "\x1b[32:1m";
//static const char* ANSI_BOLD_YELLOW = "\x1b[33:1m";
//static const char* ANSI_BOLD_WHITE = "\x1b[37;1m";

/* Dark Colors */
//static const char* ANSI_DARK_RED = "\x1b[31;2m";
//static const char* ANSI_DARK_GREEN = "\x1b[32;2m";
static const char* ANSI_DARK_CYAN = "\x1b[36;2m";
static const char* ANSI_DARK_GREY = "\x1b[37;2m";

const char* Logger::get_severity_color(LogSeverity sev)
{
    switch(sev) {
    case LSEVERITY_FATAL:
    case LSEVERITY_ERROR:
        return ANSI_RED;
    case LSEVERITY_SECURITY:
        return ANSI_ORANGE;
    case LSEVERITY_WARNING:
        return ANSI_YELLOW;
    case LSEVERITY_DEBUG:
    case LSEVERITY_PACKET:
    case LSEVERITY_TRACE:
        return ANSI_DARK_CYAN;
    case LSEVERITY_INFO:
        return ANSI_GREEN;
    default:
        return ANSI_GREY;
    }
}

// log returns an output stream for C++ style stream operations.
LockedLogOutput Logger::log(LogSeverity sev)
{
    const char *sevtext;

    if(sev < m_severity) {
        LockedLogOutput null_out(nullptr, nullptr);
        return null_out;
    }

    switch(sev) {
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

    if(m_color_enabled) {
        out << ANSI_DARK_GREY
            << "[" << timetext << "] "
            << get_severity_color(sev)
            << sevtext
            << ": "
            << ANSI_RESET;
    } else {
        out << "[" << timetext << "] "
            << sevtext
            << ": ";
    }


    return out;
}


// set_color_enabled turns ANSI colorized output on or off.
void Logger::set_color_enabled(bool enabled)
{
    m_color_enabled = enabled;
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
    std::cout << std::unitbuf;
}

LoggerBuf::LoggerBuf(const std::string &file_name, bool output_to_console) :
    m_file(file_name), m_has_file(true), m_output_to_console(output_to_console)
{
    if(m_output_to_console) {
        std::cout << std::unitbuf;
    }

    if(!m_file.is_open()) {
        m_has_file = false;
    }
}

int LoggerBuf::overflow(int c)
{
    if(m_output_to_console) {
        std::cout.put(c);
    }
    if(m_has_file) {
        m_file.put(c);
    }
    return c;
}

std::streamsize LoggerBuf::xsputn(const char* s, std::streamsize n)
{
    if(m_output_to_console) {
        std::cout.write(s, n);
    }
    if(m_has_file) {
        m_file.write(s, n);
    }
    return n;
}
