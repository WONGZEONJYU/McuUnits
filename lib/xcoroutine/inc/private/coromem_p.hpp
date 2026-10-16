#ifndef CORO_MEM_P_HPP
#define CORO_MEM_P_HPP 1

#ifndef X_COROUTINE_
#error Do not coroutine.hpp directly
#endif

#pragma once

#include <xmemory.hpp>

namespace Coro::detail {

#undef non_os
#define non_os                                                      \
    static constexpr auto allocate(std::size_t const n) noexcept    \
    { return malloc(n); }                                           \
                                                                    \
    static constexpr void deallocate(void * const p) noexcept       \
    { free(p); }

#if defined(FREERTOS) || defined(USE_FREERTOS)

#if configSUPPORT_DYNAMIC_ALLOCATION > 0

    static constexpr auto allocate(std::size_t const n) noexcept {
        taskENTER_CRITICAL();
        auto const ptr{ pvPortMalloc(n) };
        taskEXIT_CRITICAL();
        return ptr;
    }

    static constexpr void deallocate(void * const p) noexcept {
        taskENTER_CRITICAL();
        vPortFree(p);
        taskEXIT_CRITICAL();
    }
#else
    non_os
#endif

#else
    non_os
#endif

}

#undef non_os

#endif
