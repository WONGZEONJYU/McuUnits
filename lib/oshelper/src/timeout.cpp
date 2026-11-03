#include <timeout.hpp>
#include <xmemory.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>

ElapsedTimer::ElapsedTimer(std::chrono::milliseconds const ms) noexcept :
    m_waitTime_ { pdMS_TO_TICKS(ms.count()) },
    m_recordWaitTime_{ m_waitTime_ }
{ vTaskSetTimeOutState(std::addressof(m_timeOut_)); }

ElapsedTimer::ElapsedTimer(uint32_t const ms) noexcept:
    ElapsedTimer { std::chrono::milliseconds{ms} }
{   }

void ElapsedTimer::setTimeOut(uint32_t const ms) noexcept
{ setTimeOut(std::chrono::milliseconds{ms}); }

void ElapsedTimer::setTimeOut(std::chrono::milliseconds const mills) noexcept {
    auto const ms { pdMS_TO_TICKS(mills.count()) };
    m_waitTime_ = ms;
    m_recordWaitTime_ = ms;
}

void ElapsedTimer::restart() const noexcept {
    m_waitTime_ = m_recordWaitTime_;
    vTaskSetTimeOutState(std::addressof(m_timeOut_));
}

bool ElapsedTimer::timeOut() const noexcept {
    return xTaskCheckForTimeOut(std::addressof(m_timeOut_),
        std::addressof(m_waitTime_));
}

ElapsedTimer::operator bool() const noexcept
{ return timeOut(); }

TickType_t ElapsedTimer::remaining() const noexcept
{ return m_waitTime_; }

#endif
