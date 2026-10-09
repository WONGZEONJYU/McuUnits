#ifndef FCDETECTOR_TIMEOUT_HPP
#define FCDETECTOR_TIMEOUT_HPP

#include <xclasshelpermacros.hpp>
#include <xmemory.hpp>
#include <chrono>

#if defined(FREERTOS) || defined(USE_FREERTOS)

class ElapsedTimerPrivate;

class ElapsedTimer final {

    W_DECLARE_PRIVATE_D(m_d_ptr_,ElapsedTimer)
    XUniquePtr<ElapsedTimerPrivate> m_d_ptr_;

public:
    X_IMPLICIT ElapsedTimer(std::chrono::milliseconds) noexcept;
    X_IMPLICIT ElapsedTimer(uint32_t = 1000) noexcept;
    void setTimeOut(uint32_t) noexcept;
    void setTimeOut(std::chrono::milliseconds) noexcept;
    void restart() noexcept;
    [[nodiscard]] bool timeOut() noexcept;
    X_IMPLICIT operator bool() noexcept;
    ~ElapsedTimer();
};

#endif

#endif
