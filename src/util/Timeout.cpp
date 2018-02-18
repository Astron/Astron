#include "Timeout.h"
#include "core/global.h"

#include <boost/bind.hpp>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_loop(uvw::Loop::getDefault()),
    m_timer(nullptr),
    m_callback(f),
    m_timeout_interval(ms),
    m_callback_disabled(false)
{
    m_timer = m_loop->resource<uvw::TimerHandle>();
}

void Timeout::timer_callback()
{
    if(m_callback_disabled.exchange(true)) {
        return; // Stop m_callback running twice or after successful cancel().
    }

    m_callback();
}

void Timeout::reset()
{
    m_timer->stop();
    m_timer->start(uvw::TimerHandle::Time{m_timeout_interval}, uvw::TimerHandle::Time{0});
}

bool Timeout::cancel()
{
    m_timer->stop();
    return !m_callback_disabled.exchange(true);
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
}
