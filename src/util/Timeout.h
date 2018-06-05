#pragma once
#include <functional>
#include <atomic>
#include <memory>
#include "deps/uvw/uvw.hpp"
#include "util/EventQueue.h"

// This class abstracts the uvw::TimerHandle timer in order to provide a generic
// facility for timeouts. Once constructed, this class will wait a certain
// amount of time and then call the function. The timeout must be canceled
// with cancel() before you invalidate your callback.
//
// You must start the timeout with start().
//
// NOTE: The thread that calls the function is undefined. Ensure that your
// callback is thread-safe.

typedef std::function<void()> TimeoutCallback;

class Timeout
{
  public:
    Timeout(unsigned long ms, std::function<void()> f);
    Timeout();

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

    void initialize(unsigned long ms, TimeoutCallback callback);

  private:
    std::shared_ptr<uvw::Loop> m_loop;
    std::shared_ptr<uvw::TimerHandle> m_timer;
    TimeoutCallback m_callback;
    unsigned long m_timeout_interval;

    std::atomic<bool> m_callback_disabled;

    void setup();
    void destroy_timer();
    void timer_callback();
};

typedef std::function<void(Timeout*)> TimeoutSetCallback;

