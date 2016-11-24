#pragma once
#include <functional>
#include <atomic>
#include <memory>
#include <boost/asio.hpp>

// This class abstracts the boost::asio timer in order to provide a generic
// facility for timeouts. Once constructed, this class will wait a certain
// amount of time and then call the function. The timeout must be canceled
// with cancel() before you invalidate your callback.
//
// You must start the timeout with start().
//
// NOTE: The thread that calls the function is undefined. Ensure that your
// callback is thread-safe.
class Timeout : public std::enable_shared_from_this<Timeout>
{
  public:
    Timeout(unsigned long ms, std::function<void()> f);
    ~Timeout();
    inline void start()
    {
        reset();
    }
    void reset();
    // cancel() attempts to invalidate the callback and ensure that it will not
    // run. On success, returns true, guaranteeing that the callback has/will
    // not be triggered. On failure, returns false, indicating either that the
    // callback was already canceled, the callback has already finished
    // running, or the callback is (about to be) called.
    bool cancel();


  private:
    boost::asio::deadline_timer m_timer;
    std::function<void()> m_callback;
    long m_timeout_interval;

    std::atomic<bool> m_callback_disabled;

    void timer_callback(const boost::system::error_code &ec);
};
