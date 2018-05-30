#include "Timeout.h"
#include "core/global.h"

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

void Timeout::initialize(unsigned long ms, TimeoutCallback callback)
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_timeout_interval = ms;
    m_callback = callback;

    m_cancel_handle = m_loop->resource<uvw::AsyncHandle>();
    m_timer = m_loop->resource<uvw::TimerHandle>();
}

void Timeout::destroy_timer()
{
    m_callback = nullptr;
    m_cancel_handle = nullptr;
    m_timer = nullptr;
}

void Timeout::timer_callback()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_callback != nullptr && !m_callback_disabled.exchange(true)) {
        m_callback();
    }

    destroy_timer();
}

void Timeout::reset()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    m_timer->once<uvw::TimerEvent>([self = shared_from_this()](const uvw::TimerEvent&, uvw::TimerHandle&) {
        self->timer_callback();
    });

    m_cancel_handle->once<uvw::AsyncEvent>([self = shared_from_this()](const uvw::AsyncEvent&, uvw::AsyncHandle&) {
        self->cancel();
    });

    m_timer->stop();
    m_timer->start(uvw::TimerHandle::Time{m_timeout_interval}, uvw::TimerHandle::Time{0});
}

bool Timeout::cancel()
{
    const bool already_cancelled = !m_callback_disabled.exchange(true);

    if(std::this_thread::get_id() != g_main_thread_id) {
        if(m_cancel_handle != nullptr)
            m_cancel_handle->send();

        return already_cancelled;
    }

    if(m_timer != nullptr) {
        m_timer->stop();
        destroy_timer();
    }

    return already_cancelled;
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
}
