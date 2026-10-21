#ifndef CORO_MEM_P_HPP
#define CORO_MEM_P_HPP 1

#ifndef X_COROUTINE_
#error Do not coroutine.hpp directly
#endif

#pragma once

#include <xmemory.hpp>

namespace Coro::detail {

    using coroutine_handle_container = XVector<std::coroutine_handle<>>;

#undef NON_OS_ALLOCATE_DEALLOCATE
#define NON_OS_ALLOCATE_DEALLOCATE                                  \
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
    NON_OS_ALLOCATE_DEALLOCATE
#endif

#else
    NON_OS_ALLOCATE_DEALLOCATE
#endif

}

#undef NON_OS_ALLOCATE_DEALLOCATE

#endif
