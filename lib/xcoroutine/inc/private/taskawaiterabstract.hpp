#ifndef XUTILS2_TASK_AWAITER_ABSTRACT_HPP
#define XUTILS2_TASK_AWAITER_ABSTRACT_HPP 1

#ifndef X_COROUTINE_
#error Do not taskawaiterabstract.hpp directly
#endif

#pragma once
#include <xclasshelpermacros.hpp>
#include <coroutine>

namespace Coro::detail {

    template<typename Promise>
    struct TaskAwaiterAbstract {
    protected:
        using coroutine_handle_ = std::coroutine_handle<Promise>;
        coroutine_handle_ m_awaitedCoroutine_ {};

    public:
        [[nodiscard]] constexpr bool await_ready() const noexcept
        { return m_awaitedCoroutine_ && m_awaitedCoroutine_.done(); }

        constexpr void await_suspend(std::coroutine_handle<> const h) noexcept {
            if (!m_awaitedCoroutine_) { return; } // 会导致参数h这个协程序永久阻塞
            m_awaitedCoroutine_.promise().addAwaitingCoroutine(h);
        }

    protected:
        X_IMPLICIT constexpr TaskAwaiterAbstract(coroutine_handle_ const h) noexcept
            : m_awaitedCoroutine_ { h }
        {   }
    };

}

#endif
