#ifndef XUTILS2_X_CORO_LAZY_TASK_HPP
#define XUTILS2_X_CORO_LAZY_TASK_HPP 1

#pragma once

#include <xcoroutinetask.hpp>
#include <cassert>

namespace Coro {

    template<typename T> class XCoroLazyTask;

    namespace detail {

        template<typename T>
        struct is_task<XCoroLazyTask<T>> : std::true_type
        { using return_type = XCoroLazyTask<T>::value_type; };

        template<typename T>
        struct LazyTaskPromise : TaskPromise<T> {

            using TaskPromise<T>::TaskPromise;

            XCoroLazyTask<T> get_return_object() noexcept
            { return std::coroutine_handle<LazyTaskPromise>::from_promise(*this); }

            static XCoroLazyTask<T> get_return_object_on_allocation_failure() noexcept
            { std::abort(); return {}; }

            static constexpr auto initial_suspend() noexcept
            { return std::suspend_always {}; }
        };

    }

    template<typename T = void>
    class XCoroLazyTask final
        : public detail::XCoroTaskAbstract<T, XCoroLazyTask, detail::LazyTaskPromise<T>>
    {
        using Base = detail::XCoroTaskAbstract<T, XCoroLazyTask, detail::LazyTaskPromise<T>>;
        using coroutine_handle_ = Base::coroutine_handle_;

    public:
        using promise_type = detail::LazyTaskPromise<T>;
        using value_type = T;

        ~XCoroLazyTask() override {
    #ifndef NDEBUG
            if (this->m_coroutine_ && !this->m_coroutine_.done()) {
                // "XCoroLazyTask destroyed before it was awaited!"
                while (true){ }
            }
    #endif
        }

        constexpr auto operator co_await() const noexcept {

            using TaskAwaiterAbstract = detail::TaskAwaiterAbstract<promise_type>;

            class TaskAwaiter final : public TaskAwaiterAbstract {

            public:
                X_IMPLICIT constexpr TaskAwaiter(TaskAwaiterAbstract::coroutine_handle_ const h) noexcept
                    : TaskAwaiterAbstract { h }
                {   }

                constexpr auto await_suspend(std::coroutine_handle<> const h) noexcept{
                    TaskAwaiterAbstract::await_suspend(h);
                    // Return handle to the lazy task, so that it gets automatically resumed.
                    return this->m_awaitedCoroutine_;
                }

                constexpr auto await_resume(){
                    if (!this->m_awaitedCoroutine_) { abort(); }
                    if constexpr (std::is_void_v<T>) { this->m_awaitedCoroutine_.promise().result(); }
                    else { return std::move(this->m_awaitedCoroutine_.promise().result()); }
                }
            };

            return TaskAwaiter { this->m_coroutine_ };
        }

        constexpr XCoroLazyTask() noexcept = default;

        X_IMPLICIT constexpr XCoroLazyTask(coroutine_handle_ const h) noexcept
            : Base { h }
        {   }

        X_IMPLICIT constexpr XCoroLazyTask(promise_type & promise) noexcept
            : XCoroLazyTask { coroutine_handle_::from_promise(promise) }
        {   }

        X_IMPLICIT constexpr XCoroLazyTask(promise_type * const promise) noexcept
            : XCoroLazyTask { *promise }
        {   }

        W_DISABLE_COPY(XCoroLazyTask)
        W_DEFAULT_MOVE(XCoroLazyTask)
    };

    using XCoroLazyTaskVoid = XCoroLazyTask<>;

}

#endif
