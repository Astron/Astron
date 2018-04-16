#include "Timeout.h"
#include "core/global.h"

#include <boost/bind.hpp>

Timeout::Timeout(unsigned long ms, std::function<void()> f) :
    m_loop(g_loop),
    m_timer(nullptr),
    m_callback_disabled(false)
{
    initialize(ms, f);
}

Timeout::Timeout() :
    m_loop(g_loop),
    m_timer(nullptr),
    m_callback_disabled(false)
{
}

void Timeout::setup_handlers()
{
    m_reset_handle->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent&, uvw::AsyncHandle&) {
        this->reset();
    });
}

void Timeout::initialize(unsigned long ms, TimeoutCallback callback)
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_timeout_interval = ms;
    m_callback = callback;

    m_reset_handle = m_loop->resource<uvw::AsyncHandle>();
    m_cancel_handle = m_loop->resource<uvw::AsyncHandle>();
    m_timer = m_loop->resource<uvw::TimerHandle>();
    setup_handlers();
}

void Timeout::timer_callback()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_callback == nullptr) {
        return;
    }

    if(m_callback_disabled.exchange(true)) {
        return; // Stop m_callback running twice or after successful cancel().
    }

    m_callback();
}

void Timeout::reset()
{
    if(std::this_thread::get_id() != g_main_thread_id) {
        m_reset_handle->send();
        return;
    }

    m_timer->once<uvw::TimerEvent>([this](const uvw::TimerEvent&, uvw::TimerHandle&) {
        this->timer_callback();
    });

    m_cancel_handle->once<uvw::AsyncEvent>([this](const uvw::AsyncEvent&, uvw::AsyncHandle&) {
        this->cancel();
    });

    m_timer->stop();
    m_timer->start(uvw::TimerHandle::Time{m_timeout_interval}, uvw::TimerHandle::Time{0});
}

bool Timeout::cancel()
{
    const bool already_cancelled = !m_callback_disabled.exchange(true);

    if(std::this_thread::get_id() != g_main_thread_id) {
        m_cancel_handle->send();
        return already_cancelled;
    }

    m_timer->stop();
    return already_cancelled;
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
}
