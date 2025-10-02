#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP 1

#include <wglobal.hpp>
#include <cstddef>
#include <cstdlib>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <semphr.h>

template< std::ptrdiff_t LeastMaxValue = 32 >
class CountingSemaphore final {
    static_assert(LeastMaxValue >= 0, "The least maximum value must be a positive number");
    W_DISABLE_COPY(CountingSemaphore)
#if configSUPPORT_STATIC_ALLOCATION > 0
    mutable StaticSemaphore_t m_semaphore_{};
#endif
    mutable SemaphoreHandle_t m_semaphoreHandle_{};

public:
    static constexpr auto max() noexcept
    { return LeastMaxValue; }

    constexpr explicit CountingSemaphore(std::ptrdiff_t const desired = 0)
#if configSUPPORT_STATIC_ALLOCATION > 0
    :m_semaphoreHandle_ { xSemaphoreCreateCountingStatic(max(),desired,&m_semaphore_) }
#else
    :m_semaphoreHandle_ { xSemaphoreCreateCounting(max(),desired) }
#endif
    { if (desired < 0 || desired > max()) { std::abort(); } }

    constexpr ~CountingSemaphore()
    { vSemaphoreDelete(m_semaphoreHandle_); }
    /**
     * 获取信号量
     * @param wait -1永远阻塞,0不阻塞,greater than 0 等待wait毫秒
     * @return true or false
     */
    [[nodiscard]] constexpr bool acquire(int64_t const wait = -1) const noexcept
    { return static_cast<bool>(xSemaphoreTake(m_semaphoreHandle_,wait < 0 ? portMAX_DELAY : static_cast<TickType_t>(wait))); }

    [[nodiscard]] constexpr bool acquireFromISR() const noexcept {
        BaseType_t x{},b { xSemaphoreTakeFromISR( m_semaphoreHandle_,&x) };
        portYIELD_FROM_ISR(x);
        return static_cast<bool>(b);
    }

    [[nodiscard]] constexpr bool releases() const noexcept
    { return static_cast<bool>(xSemaphoreGive(m_semaphoreHandle_)); }

    [[nodiscard]] constexpr bool releasesFromISR() const noexcept {
        BaseType_t x{},b{ xSemaphoreGiveFromISR(m_semaphoreHandle_,&x) };
        portYIELD_FROM_ISR(x);
        return static_cast<bool>(b);
    }
};

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
