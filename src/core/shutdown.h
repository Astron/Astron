#pragma once
#include <exception>

// astron_handle_signals sets up signal handlers for the native OS
void astron_handle_signals();

// astron_shutdown tells astron to exit gracefully with a given error code
void astron_shutdown(int exit_code, bool throw_exception = true);

// ShutdownException is thrown by astron_shutdown to prevent
// the current thread from continuing execution.
class ShutdownException : public std::exception
{
  private:
    int m_exit_code;

  public:
    ShutdownException(int exit_code);
    int exit_code() const;
    virtual const char* what() const throw();
};
