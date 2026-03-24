#ifndef XUTILS2_TASK_PROMISE_ABSTRACT_HPP
#define XUTILS2_TASK_PROMISE_ABSTRACT_HPP 1

#ifndef X_COROUTINE_
#error Do not taskpromiseabstract.hpp directly
#endif

#pragma once

#include <xclasshelpermacros.hpp>
#include <xatomic.hpp>
#include <private/mixns.hpp>
#include <coroutine>
#include <vector>

namespace CORO::detail{

    using coroutine_handle_vector = std::vector<std::coroutine_handle<>>;

    class TaskFinalSuspend final {
        coroutine_handle_vector m_awaitingCoroutines_ {};
    public:
        X_IMPLICIT constexpr TaskFinalSuspend(coroutine_handle_vector && awaitingCoroutines)
            : m_awaitingCoroutines_ { std::move(awaitingCoroutines) }
        {   }

        static constexpr bool await_ready() noexcept
        { return {}; }

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> const h) noexcept {
            auto && promise{ h.promise() };
            for (auto && awaiter : m_awaitingCoroutines_)
            { awaiter.resume(); }
            m_awaitingCoroutines_.clear();
            promise.derefCoroutine();
        }

        static constexpr void await_resume() noexcept {}
    };

    class TaskPromiseAbstract : public AwaitTransformMixin {
        friend class TaskFinalSuspend;
        coroutine_handle_vector m_awaitingCoroutines_ {};
        XAtomicInteger<uint32_t> m_ref_ {1};

    public:
        static constexpr auto initial_suspend() noexcept
        { return std::suspend_never {}; }

        constexpr auto final_suspend() noexcept
        { return TaskFinalSuspend {std::move(m_awaitingCoroutines_) }; }

        constexpr void addAwaitingCoroutine(std::coroutine_handle<> const awaitingCoroutine) noexcept
        { m_awaitingCoroutines_.push_back(awaitingCoroutine); }

        [[nodiscard]] constexpr bool hasAwaitingCoroutine() const noexcept
        { return !m_awaitingCoroutines_.empty(); }

        void derefCoroutine() noexcept
        { if (!m_ref_.deref()) { destroyCoroutine(); } }

        void refCoroutine() noexcept
        { m_ref_.ref(); }

        void destroyCoroutine() noexcept{
            m_ref_.storeRelaxed({});
            auto const handle { std::coroutine_handle<TaskPromiseAbstract>::from_promise(*this) };
            handle.destroy();
        }

        constexpr virtual ~TaskPromiseAbstract() noexcept = default;

    protected:
        constexpr TaskPromiseAbstract() noexcept = default;
    };

}

#endif
