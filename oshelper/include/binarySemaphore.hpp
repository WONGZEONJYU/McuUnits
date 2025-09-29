#pragma once
#ifndef BINARY_SEMAPHORE_HPP_
#define BINARY_SEMAPHORE_HPP_ 1

#include <wglobal.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

#include <FreeRTOS.h>
#include <semphr.h>

class BinarySemaphore final {
    W_DISABLE_COPY_MOVE(BinarySemaphore)
#if configSUPPORT_STATIC_ALLOCATION > 0
    mutable StaticSemaphore_t m_semaphore_{};
#endif
    mutable SemaphoreHandle_t m_semaphoreHandle_{};

public:
    explicit BinarySemaphore();
    ~BinarySemaphore();
    /**
     * 获取信号量
     * @param wait -1永远阻塞,0不阻塞,greater than 0 等待wait毫秒
     * @return true or false
     */
    [[nodiscard]] bool acquire(int64_t wait = -1) const noexcept;
    [[nodiscard]] bool acquireFromISR() const noexcept;
    [[nodiscard]] bool releases() const noexcept;
    [[nodiscard]] bool releasesFromISR() const noexcept;
};

#endif
#endif
