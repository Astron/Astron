#include "Timeout.h"
#include "core/global.h"

#include <boost/bind.hpp>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_timer(io_service, boost::posix_time::milliseconds(ms)),
    m_callback(f),
    m_timeout_interval(ms),
    m_callback_disabled(false)
{
}

void Timeout::timer_callback(const boost::system::error_code &ec)
{
    if(ec) {
        return; // We were canceled.
    }

    if(m_callback_disabled.exchange(true)) {
        return; // Stop m_callback running twice or after successful cancel().
    }

    m_callback();
}

void Timeout::reset()
{
    m_timer.cancel();
    m_timer.expires_from_now(boost::posix_time::millisec(m_timeout_interval));
    m_timer.async_wait(boost::bind(&Timeout::timer_callback, shared_from_this(),
                                   boost::asio::placeholders::error));
}

bool Timeout::cancel()
{
    m_timer.cancel();
    return !m_callback_disabled.exchange(true);
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
}
