#ifndef X_TUPLE_HELPER_HPP
#define X_TUPLE_HELPER_HPP 1

#include <xtypetraits.hpp>

/**
 * 遍历tuple所有元素
 * Pred参数为一个函数,
 * Pred参数有(parm1:std::size & index,
 *  parm2:遍历的元素类型,一般用auto)
 * @tparam Tuple
 * @tparam Pred
 * @param tuple_
 * @param pred_
 */
template<typename Tuple, typename Pred> requires (is_tuple_v<Tuple>)
[[maybe_unused]] constexpr void for_each_tuple(Tuple && tuple_, Pred && pred_)
noexcept( std::is_nothrow_invocable_v<Pred> )
{
    (void)std::apply([&pred_]<typename... Args>(Args &&... args) {
        std::size_t index{};
        (std::forward<Pred>(pred_)(std::ref(index),std::forward<Args>(args)), ...);
    },std::forward<Tuple>(tuple_));
}

namespace TuplePrivate {

    template<std::size_t N>
    inline constexpr auto indices { std::make_index_sequence<N>{} };

    template<const std::size_t N,typename... Args>
    constexpr auto Left_Tuple_n_(std::tuple<Args...> const & tuple_) noexcept {
        static_assert(N <= sizeof...(Args), "N must be <= tuple size");
        static_assert(N <= std::tuple_size_v<std::decay_t<decltype(tuple_)>>, "N must be <= tuple size");
        return [&]<std::size_t... I>(std::index_sequence<I...>) noexcept
            -> decltype(std::make_tuple(std::get<I>(tuple_)...))
        {
            return std::make_tuple(std::get<I>(tuple_)...);
        }(indices<N>);
    }

    // 方法1: 获取后N个元素的通用模板
    template<const std::size_t N, typename... Args>
    constexpr auto Last_Tuple_n_(std::tuple<Args...> const & tuple_) noexcept {
        static_assert(N <= sizeof...(Args), "N must be <= tuple size");
        static_assert(N <= std::tuple_size_v<std::decay_t<decltype(tuple_)>>,"N must be <= tuple size");
        constexpr auto S{ sizeof...(Args)  - N };
        return [&]<std::size_t... I>(std::index_sequence<I...>) noexcept
            ->decltype(std::make_tuple(std::get<S + I>(tuple_)...))
        {
            return std::make_tuple(std::get<S + I>(tuple_)...);
        }(indices<N>);
    }

    // 方法3: 跳过前面元素,取剩余的
    template<std::size_t Skip, typename... Args>
    constexpr auto skip_front_n_(std::tuple<Args...> const & tuple_) noexcept {
        static_assert(Skip <= sizeof...(Args), "Skip count exceeds tuple size");
        constexpr auto Offset{Skip},ReCount{sizeof...(Args) - Skip};
        static_assert(Offset + ReCount <= sizeof...(Args), "Range out of bounds");
        return [&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
        -> decltype(std::make_tuple(std::get<Offset + I>(tuple_)...))
        {
            return std::make_tuple(std::get<Offset + I>(tuple_)...);
        }(indices<ReCount>);
    }
}

/**
 * 获取tuple前N个元素
 * @tparam N
 * @tparam Tuple
 * @param tuple_
 * @return
 */
template<const std::size_t N,typename Tuple> requires (is_tuple_v<Tuple>)
constexpr auto Left_Tuple(Tuple const & tuple_) noexcept
{ return TuplePrivate::Left_Tuple_n_<N>(tuple_); }

/**
 * 获取tuple后N个元素
 * @tparam N
 * @tparam Tuple
 * @param tuple_
 * @return decltype(TuplePrivate::Last_Tuple_n_<N,Tuple>(tuple_))
 */
template<const std::size_t N,typename Tuple> requires (is_tuple_v<Tuple>)
constexpr auto Last_Tuple(Tuple const & tuple_) noexcept
{ return TuplePrivate::Last_Tuple_n_<N>(tuple_); }

/**
 * 获取tuple跳过N个元素后面的所有元素
 * @tparam N
 * @tparam Tuple
 * @param tuple_
 * @return decltype(TuplePrivate::skip_front_n_<Skip>(tuple_))
 */
template<const std::size_t N,typename Tuple> requires (is_tuple_v<Tuple>)
constexpr auto SkipFront_Tuple(Tuple const & tuple_) noexcept
{ return TuplePrivate::skip_front_n_<N>(tuple_); }

#endif
