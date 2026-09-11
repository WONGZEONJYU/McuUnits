#ifndef X_RAII_HPP
#define X_RAII_HPP 1

#include <xclasshelpermacros.hpp>
#include <functional>
#include <utility>

#if 1

/**
 * 这里为什么这样绕一层呢,是防止编译器一些警告
 */
template<typename ...Args>
auto bind(Args && ...args) -> decltype(std::bind(std::forward<Args>(args)...))
{ return std::bind(std::forward<Args>(args)...); }

#else

template <typename ...Args>
auto bind(Args && ...args)
{ return [&args...]{ return std::invoke(std::forward<Args>(args)...); }; }

#endif

template<typename Fn>
class AutoDestroyer {

    Fn m_fn_{};
    mutable bool m_is_destroy{};

public:
    /**
     * 如果需参数,请使用 XUtils::bind(...) 或 std::bind(...)
     * Destroyer d {  XUtils::bing([](int){},1)  };
     */
    constexpr explicit AutoDestroyer(Fn && f) noexcept
        : m_fn_ { std::move(f) }
    {   }

    constexpr AutoDestroyer(AutoDestroyer && other) noexcept
        :m_fn_ { std::move(other.m_fn_) }
    ,m_is_destroy { std::exchange(other.m_is_destroy,{}) }
    {   }

    constexpr void destroy() const noexcept {
        if (m_is_destroy) { return; }
        m_is_destroy = true;
        m_fn_();
    }

    constexpr void dismiss() const noexcept
    { m_is_destroy = true; }

    ~AutoDestroyer() noexcept
    { destroy(); }

    W_DISABLE_COPY(AutoDestroyer)
};

template <typename F> AutoDestroyer(F(&)()) -> AutoDestroyer<F(*)()>;

template<typename Release>
class XScopeGuard final : public AutoDestroyer<Release> {
    using Base = AutoDestroyer<Release>;

public:
    template<typename Fn>
    explicit constexpr XScopeGuard(Fn && fn,Release && release) noexcept
        : Base { std::move(release) }
    { std::forward<Fn>(fn)(); }

    constexpr XScopeGuard(XScopeGuard &&) noexcept = default;

    W_DISABLE_COPY(XScopeGuard)
};

template <typename Fn, typename F>
XScopeGuard(Fn&&, F(&)()) -> XScopeGuard<F(*)()>;

#endif
