#include "Timeout.h"

#include <assert.h>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_callback(f),
    m_timeout_interval(ms),
    m_callback_disabled(false)
{
    auto loop = uvw::Loop::getDefault();
    m_timer = loop->resource<uvw::TimerHandle>();
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
    m_timer->clear();
    m_timer->stop();

    m_timer->on<uvw::TimerEvent>([ptr = shared_from_this()](const auto&, const auto&) {
        ptr->timer_callback();
    });
    m_timer->start(std::chrono::milliseconds{m_timeout_interval}, std::chrono::milliseconds{0});
}

bool Timeout::cancel()
{
    m_timer->stop();
    m_timer->clear();
    return !m_callback_disabled.exchange(true);
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
    m_timer->close();
}
