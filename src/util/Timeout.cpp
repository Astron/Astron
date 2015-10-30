#include "Timeout.h"
#include "core/global.h"

#include <boost/bind.hpp>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_timer(io_service, boost::posix_time::milliseconds(ms)),
    m_callback(f),
    m_timeout_interval(ms)
{
    m_timer.async_wait(boost::bind(&Timeout::timer_callback, this,
                boost::asio::placeholders::error));
}

void Timeout::timer_callback(const boost::system::error_code &ec)
{
    if(ec) {
        return; // We were canceled.
    }

    m_callback();
}

void Timeout::reset()
{
    m_timer.cancel();
    m_timer.expires_from_now(boost::posix_time::millisec(m_timeout_interval));
    m_timer.async_wait(boost::bind(&Timeout::timer_callback, this,
                                   boost::asio::placeholders::error));
}

Timeout::~Timeout()
{
    m_timer.cancel();
}
