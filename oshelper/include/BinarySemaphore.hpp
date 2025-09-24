#pragma once
#ifndef BINARY_SEMAPHORE_HPP_
#define BINARY_SEMAPHORE_HPP_ 1

#include <wglobal.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

#include <FreeRTOS.h>
#include <semphr.h>

class BinarySemaphore final {
    W_DISABLE_COPY_MOVE(BinarySemaphore)
    mutable SemaphoreHandle_t m_BinarySemaphore{};
#if configSUPPORT_STATIC_ALLOCATION > 0
    mutable StaticSemaphore_t m_SemaphoreBuffer{};
#endif

public:
    explicit BinarySemaphore();
    ~BinarySemaphore();
    [[nodiscard]] bool acquire(int64_t wait = -1) const noexcept;
    [[nodiscard]] bool acquireFromISR() const noexcept;
    [[nodiscard]] bool releases() const noexcept;
    [[nodiscard]] bool releasesFromISR() const noexcept;
};

#endif
#endif
