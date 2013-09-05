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

class NullStream {
public:
	void setFile(){/* no-op */}
	template<typename TPrintable>
	NullStream& operator<<(TPrintable const&) {/* no-op */}
};
NullStream null_stream;

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

extern Logger *gLogger;

class LogCategory {
public:
	LogCategory(const std::string &id, const std::string &name) : m_id(id), m_name(name)
	{
	}

	LogCategory(const char *id, const std::string &name) : m_id(id), m_name(name)
	{
	}

	LogCategory(const char *id, const char *name) : m_id(id), m_name(name)
	{
	}

#define F(level) \
	std::ostream &level() \
	{ \
		std::ostream &out = gLogger->level(); \
		out << m_name << ": "; \
		return out; \
	}

#ifdef DEBUG_MESSAGES
	F(spam)
	F(debug)
#else
	inline NullStream &spam() { return null_stream; }
	inline NullStream &debug() { return null_stream; }
#endif
	F(info)
	F(warning)
	F(security)
	F(error)
	F(fatal)

#undef F

private:
	std::string m_id;
	std::string m_name;
};
