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

enum LogSeverity
{
    LSEVERITY_PACKET,
    LSEVERITY_SPAM,
    LSEVERITY_DEBUG,
    LSEVERITY_INFO,
    LSEVERITY_WARNING,
    LSEVERITY_SECURITY,
    LSEVERITY_ERROR,
    LSEVERITY_FATAL
};

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
		Logger(const std::string &log_file, bool console_output = true);
		Logger();

		std::ostream &log(LogSeverity sev);

	private:
		LoggerBuf m_buf;
		std::ostream m_output;
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


#ifdef PACKET_DEBUG
    F(packet, LSEVERITY_PACKET)
#else
    inline NullStream &packet()
    {
        return null_stream;
    }
#endif

#ifdef DEBUG_MESSAGES
		F(spam, LSEVERITY_SPAM)
		F(debug, LSEVERITY_DEBUG)
#else
		inline NullStream &spam()
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
