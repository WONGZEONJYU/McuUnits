#include <coroawaiter.hpp>
#include <timeout.hpp>
#include <event.hpp>

CoroAwaiter::CoroAwaiter(uint32_t const timeout)
    :m_waitTime_{ timeout }
{   }

CoroAwaiter::CoroAwaiter(std::chrono::milliseconds const timeout)
    :CoroAwaiter{static_cast<uint32_t>(timeout.count())}
{   }

void CoroAwaiter::await_suspend(std::coroutine_handle<> const h) const noexcept {
    if (m_waitTime_ <= 0) { h.resume(); return; }
    EventLoop::postTimeEvent(m_waitTime_,[h]()noexcept{h.resume();});
}

CoroAwaiter::~CoroAwaiter() = default;
