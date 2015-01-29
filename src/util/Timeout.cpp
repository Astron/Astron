#include "Timeout.h"
#include "core/global.h"

#include <boost/bind.hpp>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_timer(io_service, boost::posix_time::milliseconds(ms)),
    m_callback(f)
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

Timeout::~Timeout()
{
    m_timer.cancel();
}
