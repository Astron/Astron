#include <time.h>
#include <iostream>
#include <fstream>

#include "Logger.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

CONSOLE_SCREEN_BUFFER_INFO old_colors;
#endif

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

#ifdef _WIN32
#define STDOUT GetStdHandle(STD_OUTPUT_HANDLE)
#define RESET (SetConsoleTextAttribute(STDOUT, old_colors.wAttributes) ? "" : "")

#define RED (SetConsoleTextAttribute(STDOUT, FOREGROUND_RED) ? "" : "")
#define GREEN (SetConsoleTextAttribute(STDOUT, FOREGROUND_GREEN) ? "" : "")
#define ORANGE (SetConsoleTextAttribute(STDOUT, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY) ? "" : "") // This is actually light magenta; there's no orange.
#define YELLOW (SetConsoleTextAttribute(STDOUT, FOREGROUND_RED | FOREGROUND_GREEN) ? "" : "")
#define GREY (SetConsoleTextAttribute(STDOUT, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) ? "" : "")
#define CYAN (SetConsoleTextAttribute(STDOUT, FOREGROUND_GREEN | FOREGROUND_BLUE) ? "" : "")

#define DARK_GREY (SetConsoleTextAttribute(STDOUT, FOREGROUND_INTENSITY) ? "" : "")
#else
#define RESET "\x1b[0m"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define ORANGE "\x1b[33m"
#define YELLOW "\x1b[33;2m"
#define GREY "\x1b[37m"
#define CYAN "\x1b[36;2m"

#define DARK_GREY "\x1b[37;2m"
#endif

auto Logger::get_severity_color(LogSeverity sev)
{

    switch(sev) {
    case LSEVERITY_FATAL:
    case LSEVERITY_ERROR:
        return RED;
    case LSEVERITY_SECURITY:
        return ORANGE;
    case LSEVERITY_WARNING:
        return YELLOW;
    case LSEVERITY_DEBUG:
    case LSEVERITY_PACKET:
    case LSEVERITY_TRACE:
        return CYAN;
    case LSEVERITY_INFO:
        return GREEN;
    default:
        return GREY;
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
#ifdef _WIN32
		GetConsoleScreenBufferInfo(STDOUT, &old_colors);
#endif
		out << DARK_GREY;
		out << "[" << timetext << "] ";
		out << get_severity_color(sev);
		out << sevtext;
		out << ": ";
		out << RESET;

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
