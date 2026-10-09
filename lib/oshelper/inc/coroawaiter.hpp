#ifndef CORO_AWAITER_HPP
#define CORO_AWAITER_HPP 1

#include <xmemory.hpp>
#include <coroutine>

class CoroDelayPrivate;

class CoroDelay final {

    XUniquePtr<CoroDelayPrivate> m_d_ptr_;
    W_DISABLE_COPY_MOVE(CoroDelay)
    W_DECLARE_PRIVATE_D(m_d_ptr_,CoroDelay)

public:
    explicit CoroDelay();
    ~CoroDelay();
    void exec();
    void quit();
    friend class CoroAwaiter;
};

class CoroAwaiterPrivate;

class CoroAwaiter final {

    XUniquePtr<CoroAwaiterPrivate> m_d_ptr_;
    W_DISABLE_COPY_MOVE(CoroAwaiter)
    W_DECLARE_PRIVATE_D(m_d_ptr_,CoroAwaiter)

public:
    explicit CoroAwaiter(CoroDelay &,uint32_t);
    explicit CoroAwaiter(CoroDelay &,
        std::chrono::milliseconds = std::chrono::milliseconds{1000});
    ~CoroAwaiter();

    static constexpr bool await_ready() noexcept
    { return {}; }

    void await_suspend(std::coroutine_handle<>) noexcept;
    static constexpr void await_resume() noexcept {}
};

#endif
