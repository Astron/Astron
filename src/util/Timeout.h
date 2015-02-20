#pragma once
#include <functional>
#include <boost/asio.hpp>

// This class abstracts the boost::asio timer in order to provide a generic
// facility for timeouts. Once constructed, this class will wait a certain
// amount of time and then call the function. The timeout is automatically
// canceled when this class is destructed.
//
// NOTE: The thread that calls the function is undefined. Ensure that your
// callback is thread-safe.
class Timeout
{
  public:
    Timeout(unsigned long ms, std::function<void()> f);
    virtual ~Timeout();
    void reset();
    

  private:
    boost::asio::deadline_timer m_timer;
    std::function<void()> m_callback;
    long m_timeout_interval;
    
    void timer_callback(const boost::system::error_code &ec);
};
