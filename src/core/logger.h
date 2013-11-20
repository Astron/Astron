#pragma once
#include <stdarg.h>
#include <string>
#include <iostream>
#include <fstream>

class LoggerBuf : public std::streambuf
{
	public:
		LoggerBuf();
		LoggerBuf(const std::string &file_name, bool output_to_console = true);
	protected:
		int overflow(int c = EOF);
		std::streamsize xsputn(const char* s, std::streamsize n);
	private:
		std::ofstream m_file;
		bool m_has_file;
		bool m_output_to_console;
};

// NullBuffer is used to make streams that log in cases when we do not
// want to compile the message out (for example, when the LogServity is configurable)
class NullBuffer;
extern NullBuffer null_buffer;
class NullBuffer : public std::streambuf
{
	public:
		int overflow(int c) { return c; }
};

enum LogSeverity
{
    LSEVERITY_PACKET,
    LSEVERITY_TRACE,
    LSEVERITY_DEBUG,
    LSEVERITY_INFO,
    LSEVERITY_WARNING,
    LSEVERITY_SECURITY,
    LSEVERITY_ERROR,
    LSEVERITY_FATAL
};

// NullStream is used with pre-processor definitions to define a stream that when
// logged to, will be reduced to a no-op and compiled out.
class NullStream;
extern NullStream null_stream;

class NullStream
{
	public:
		void setFile()
		{
			/* no-op */
		}
		template<typename TPrintable>
		inline NullStream& operator<<(TPrintable const&)
		{
			return null_stream;
		}
		inline NullStream& operator<<(std::ostream & (std::ostream&))
		{
			return null_stream;
		}
};

class Logger
{
	public:
		Logger(const std::string &log_file, LogSeverity sev, bool console_output = true);
		Logger();

		std::ostream &log(LogSeverity sev);

		void set_min_severity(LogSeverity sev);
		LogSeverity get_min_severity();

	private:
		LoggerBuf m_buf;
		LogSeverity m_severity;
		std::ostream m_output;
		std::ostream m_null;
};

extern Logger *g_logger;

class LogCategory
{
	public:
		LogCategory(const std::string &id, const std::string &name) : m_id(id), m_name(name)
		{
		}

		LogCategory(const char* id, const std::string &name) : m_id(id), m_name(name)
		{
		}

		LogCategory(const char* id, const char* name) : m_id(id), m_name(name)
		{
		}

		void set_name(const std::string &name)
		{
			m_name = name;
		}

#define F(level, severity) \
	std::ostream &level() \
	{ \
		std::ostream &out = g_logger->log(severity); \
		out << m_name << ": "; \
		return out; \
	}

#ifdef ASTRON_DEBUG_MESSAGES
        F(packet, LSEVERITY_PACKET)
		F(trace, LSEVERITY_TRACE)
		F(debug, LSEVERITY_DEBUG)
#else
        inline NullStream &packet()
        {
            return null_stream;
        }
		inline NullStream &trace()
		{
			return null_stream;
		}
		inline NullStream &debug()
		{
			return null_stream;
		}

#endif
		F(info, LSEVERITY_INFO)
		F(warning, LSEVERITY_WARNING)
		F(security, LSEVERITY_SECURITY)
		F(error, LSEVERITY_ERROR)
		F(fatal, LSEVERITY_FATAL)

#undef F

	private:
		std::string m_id;
		std::string m_name;
};
