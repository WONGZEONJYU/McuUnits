#include <coroawaiter.hpp>
#include <timeout.hpp>
#include <xcontainer.hpp>
#include <functional>
#include <coroutine>
#include <cassert>
#include <mutex.hpp>
#include <xthread.hpp>

class CoroDelayPrivate {

public:
    using Callback_t = std::function<void()>;
    using ElapsedTimerPtr = XUniquePtr<ElapsedTimer>;

    CoroDelay * q_ptr{};
    XUnordered_map<ElapsedTimerPtr,Callback_t> m_delayMap{};
    Mutex m_mtx{};
    bool m_running {};

    W_DECLARE_PUBLIC(CoroDelay)

    explicit CoroDelayPrivate(CoroDelay * q):q_ptr{q} {}
    ~CoroDelayPrivate() = default;

    void run() noexcept {

        XVector<Callback_t> cbList{};
        {
            std::unique_lock lk{m_mtx};
            for (auto it { m_delayMap.begin() }; it != m_delayMap.cend();) {
                auto && [k,v] { *it };
                if (!*k) { ++it;continue; }
                cbList.push_back(std::move(v));
                it = m_delayMap.erase(it);
            }
        }

        for (auto && item : cbList)
        { item(); }
    }

    void add(ElapsedTimerPtr && el,Callback_t && cb) noexcept {
        if (!el || *el || !cb ) { return; }
        std::unique_lock lk {m_mtx};
        m_delayMap[std::move(el)] = std::move(cb);
    }

    friend class CoroAwaiterPrivate;
};

CoroDelay::CoroDelay()
    : m_d_ptr_{ makeUnique<CoroDelayPrivate>(this) }
{ assert(m_d_ptr_); }

CoroDelay::~CoroDelay() = default;

void CoroDelay::exec() {
    W_D(CoroDelay);
    if (d->m_running) { return; }
    d->m_running = true;
    while (d->m_running)
    { d->run(); XAbstractThread::sleep_for(1); }
}

void CoroDelay::quit()
{ d_func()->m_running = {}; }

class CoroAwaiterPrivate {

public:
    CoroAwaiter * q_ptr{};
    W_DECLARE_PUBLIC(CoroAwaiter)

    CoroDelayPrivate * m_delay{};
    uint32_t m_waitTime{};

    explicit CoroAwaiterPrivate(CoroAwaiter * q):q_ptr{q} {}
    ~CoroAwaiterPrivate() = default;

    void suspend(std::coroutine_handle<> const h) noexcept {
        auto elapsedTime{ makeUnique<ElapsedTimer>(m_waitTime) };
        if (!elapsedTime || *elapsedTime) { h.resume(); return; }
        m_delay->add(std::move(elapsedTime),[h]{ h.resume(); });
    }
};

CoroAwaiter::CoroAwaiter(CoroDelay & delay,uint32_t const timeout):
m_d_ptr_{makeUnique<CoroAwaiterPrivate>(this)}  {
    assert(m_d_ptr_);
    W_D(CoroAwaiter);
    d->m_delay = delay.m_d_ptr_.get();
    d->m_waitTime = timeout;
}

CoroAwaiter::CoroAwaiter(CoroDelay &delay,std::chrono::milliseconds const timeout):
CoroAwaiter{delay,static_cast<long unsigned int>(timeout.count())}
{   }

void CoroAwaiter::await_suspend(std::coroutine_handle<> const h) noexcept
{ d_func()->suspend(h); }

CoroAwaiter::~CoroAwaiter() = default;
