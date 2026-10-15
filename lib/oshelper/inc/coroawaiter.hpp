#ifndef CORO_AWAITER_HPP
#define CORO_AWAITER_HPP 1

#include <xmemory.hpp>
#include <coroutine>

class CoroAwaiter final : public std::suspend_always {
    uint32_t m_waitTime_{};

public:
    explicit CoroAwaiter(uint32_t);
    explicit CoroAwaiter(std::chrono::milliseconds =
        std::chrono::milliseconds{1000});
    ~CoroAwaiter();
    void await_suspend(std::coroutine_handle<>) const noexcept;
};

#endif
