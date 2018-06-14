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

    m_timer->on<uvw::TimerEvent>([self = this](const uvw::TimerEvent&, uvw::TimerHandle&) {
        self->timer_callback();
    });
}

void Timeout::destroy_timer()
{
    m_callback = nullptr;

    if(m_timer) {
        m_timer->stop();
        m_timer->close();
        m_timer = nullptr;
    }

    delete this;
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
        TaskQueue::singleton.enqueue_task([self = this]() {
            self->cancel();
        });

        return already_cancelled;
    }

    if(m_timer != nullptr) {
        destroy_timer();
    }

    return already_cancelled;
}

Timeout::~Timeout()
{
    assert(m_callback_disabled.load());
}
