#ifndef TIMEOUT_HPP
#define TIMEOUT_HPP

#include <xclasshelpermacros.hpp>
#include <xmemory.hpp>
#include <chrono>

#if defined(FREERTOS) || defined(USE_FREERTOS)

class ElapsedTimer final {
    TimeOut_t m_timeOut_{};
    TickType_t m_waitTime_{},m_recordWaitTime_{};

public:
    X_IMPLICIT ElapsedTimer(std::chrono::milliseconds) noexcept;
    X_IMPLICIT ElapsedTimer(uint32_t = 1000) noexcept;
    ~ElapsedTimer();
    void setTimeOut(uint32_t) noexcept;
    void setTimeOut(std::chrono::milliseconds) noexcept;
    void restart() noexcept;
    [[nodiscard]] bool timeOut() noexcept;
    X_IMPLICIT operator bool() noexcept;
    [[nodiscard]] TickType_t remaining() const noexcept;
};

#endif

#endif
