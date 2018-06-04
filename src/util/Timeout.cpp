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
}

void Timeout::setup()
{
    assert(m_timer == nullptr);

    m_timer = m_loop->resource<uvw::TimerHandle>();
    m_cancel_handle = m_loop->resource<uvw::AsyncHandle>();

    std::weak_ptr<Timeout> lambda_handle = std::weak_ptr<Timeout>(shared_from_this());
    m_timer->on<uvw::TimerEvent>([self = lambda_handle](const uvw::TimerEvent&, uvw::TimerHandle&) {
        if(auto timer = self.lock())
            timer->timer_callback();
    });

    m_cancel_handle->on<uvw::AsyncEvent>([self = lambda_handle](const uvw::AsyncEvent&, uvw::AsyncHandle&) {
        if(auto timer = self.lock())
            timer->cancel();
    });
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

    if(m_timer == nullptr) {
        setup();
    }

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
