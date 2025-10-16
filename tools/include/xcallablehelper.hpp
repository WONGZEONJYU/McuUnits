#ifndef X_CALLABLE_HELPER_HPP
#define X_CALLABLE_HELPER_HPP 1

#include <xmemory.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <functional>
#include <memory>

class XCallableHelper {

    struct XFactoryCallable;

    template<typename > class XCallable;

    class XAbstractCallable {
        friend struct XFactoryCallable;

        template<typename > friend class XCallable;

        enum class Private{};

        constexpr XAbstractCallable() = default;

    public:
        constexpr virtual void operator()() const = 0;
        constexpr virtual ~XAbstractCallable() = default;
    };

    template<typename Callable>
    class XCallable final: public XAbstractCallable {
        mutable Callable m_callable_{};

    public:
        constexpr XCallable(Callable && call,Private)
        :m_callable_{std::forward<Callable>(call)}{}

        constexpr ~XCallable() override = default;

        constexpr void operator()() const override { m_callable_(); }
    };

    using CallablePtr_ = std::shared_ptr<XAbstractCallable>;

    struct XFactoryCallable{
        XFactoryCallable() = delete;
        template<typename Callable_>
        static constexpr auto create(Callable_ && call) noexcept -> CallablePtr_ {
            using XCallable_t = XCallable<Callable_>;
            return makeShared<XCallable_t>(std::forward<Callable_>(call),XAbstractCallable::Private{});
        }
    };

    struct Factory;

    template<typename Tuple>
    class XInvoker final {

        friend struct Factory;

        enum class Private_{};

        mutable Tuple m_fnAndArgs_{};

        template<typename> struct result_{};

        template<typename Fn, typename... Args>
        struct result_<std::tuple<Fn, Args...>> : std::invoke_result<Fn, Args...>{};

        template<size_t... Ind>
        constexpr result_<Tuple>::type M_invoke_(std::index_sequence<Ind...>) const
        { return std::invoke(std::get<Ind>(std::forward<decltype(m_fnAndArgs_)>(m_fnAndArgs_))...); }

    public:
        using result_t = result_<Tuple>::type;

        constexpr XInvoker(Tuple && t,Private_) noexcept
        :m_fnAndArgs_{std::forward<Tuple>(t)}{}

        constexpr result_t operator()() const {
            using Indices_ = std::make_index_sequence<std::tuple_size_v<Tuple>>;
            return M_invoke_(Indices_{});
        }
    };

    template<typename... Tp>
    using decayedTuple_ = std::tuple<std::decay_t<Tp>...>;

    struct Factory final {
        Factory() = delete;

        template<typename... Args>
        static constexpr auto createInvoker(Args && ...args) noexcept {
            using invoker_t = Invoker<Args...>;
            return invoker_t { {std::forward<Args>(args)...},typename invoker_t::Private_{} };
        }

        template<typename... Args>
        static constexpr auto createCallable(Args && ...args) noexcept -> CallablePtr_ {
            auto invoker { createInvoker(std::forward<Args>(args)...) };
            return XFactoryCallable::create(std::forward<decltype(invoker)>(invoker));
        }
    };

protected:
    explicit XCallableHelper() = default;

public:
    using CallablePtr = CallablePtr_;

    template<typename... Args>
    using Invoker = XInvoker<decayedTuple_<Args...>>;

    constexpr virtual ~XCallableHelper() = default;

    template<typename... Args>
    static constexpr auto createInvoker(Args && ...args) noexcept
    { return Factory::createInvoker(std::forward<Args>(args)...); }

    template<typename... Args>
    static constexpr auto createCallable(Args && ...args) noexcept -> CallablePtr
    { return Factory::createCallable(std::forward<Args>(args)...); }
};

#endif
