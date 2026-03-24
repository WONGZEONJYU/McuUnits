#ifndef XUTILS2_CONCEPTS_P_HPP
#define XUTILS2_CONCEPTS_P_HPP

#ifndef X_COROUTINE_
#error Do not concepts_p.hpp directly
#endif

#pragma once

#include <concepts>

namespace concepts {
    template<typename T>
    concept destructible = std::is_nothrow_destructible_v<T>;

    template<typename T, typename ... Args>
    concept constructible_from = destructible<T>
        && std::is_constructible_v<T, Args...>;
}

#endif
