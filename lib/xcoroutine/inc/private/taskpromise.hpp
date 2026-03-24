#ifndef XUTILS2_TASK_PROMISE_HPP
#define XUTILS2_TASK_PROMISE_HPP 1

#ifndef X_COROUTINE_
#error Do not taskpromise.hpp directly
#endif

#pragma once

#include <private/taskpromiseabstract.hpp>
#include <cassert>
#include <variant>
#include <exception>

namespace CORO {

template<typename T> class XCoroTask;

namespace detail {

    template<typename T>
    class TaskPromise: public TaskPromiseAbstract {
        std::variant<std::monostate, T, std::exception_ptr> m_value_ {};
    public:
        constexpr XCoroTask<T> get_return_object() noexcept
        { return { this }; }

        void unhandled_exception() noexcept
        { m_value_ = std::current_exception(); }

        constexpr void return_value(T && value) noexcept
        { m_value_.template emplace<T>(std::forward<T>(value)); }

        constexpr void return_value(T const & value) noexcept
        { m_value_ = value; }

        template<typename U> requires std::constructible_from<T, U>
        constexpr void return_value(U && value) noexcept
        { m_value_ = T(std::forward<U>(value)); }

#define HAS_EXCEPTION() do { \
        if (std::holds_alternative<std::exception_ptr>(m_value_)) { \
            assert(std::get<std::exception_ptr>(m_value_)); \
            std::rethrow_exception(std::get<std::exception_ptr>(m_value_)); \
        } } while (false)

        constexpr T & result() & noexcept
        { HAS_EXCEPTION(); return std::get<T>(m_value_); }

        constexpr T && result() && noexcept
        { HAS_EXCEPTION(); return std::move(std::get<T>(m_value_)); }

#undef HAS_EXCEPTION

        constexpr ~TaskPromise() override = default;
    };

    template<>
    class TaskPromise<void> : public TaskPromiseAbstract {
        std::exception_ptr m_exception_ {};
    public:
        constexpr XCoroTask<void> get_return_object() noexcept;

        void unhandled_exception()
        { m_exception_ = std::current_exception(); }

        static constexpr void return_void() noexcept {}

        void result() const
        { if (m_exception_) { std::rethrow_exception(m_exception_); } }

        ~TaskPromise() override = default;
    };

    using TaskPromiseVoid = TaskPromise<void>;

}

}

#endif
