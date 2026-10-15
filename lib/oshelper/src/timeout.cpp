#include <timeout.hpp>
#include <xmemory.hpp>
#include <cassert>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>

ElapsedTimer::ElapsedTimer(std::chrono::milliseconds const ms) noexcept :
m_waitTime_ { static_cast<TickType_t>(ms.count()) },m_recordWaitTime_{ m_waitTime_ }
{ vTaskSetTimeOutState(std::addressof(m_timeOut_)); }

ElapsedTimer::ElapsedTimer(uint32_t const ms) noexcept:
    ElapsedTimer { std::chrono::milliseconds{ms} }
{   }

void ElapsedTimer::setTimeOut(uint32_t const ms) noexcept {
    m_waitTime_ = pdMS_TO_TICKS(ms);
    m_recordWaitTime_ = m_waitTime_;
}

void ElapsedTimer::setTimeOut(std::chrono::milliseconds const ms) noexcept
{ setTimeOut(ms.count()); }

void ElapsedTimer::restart() noexcept {
    vTaskSetTimeOutState(std::addressof(m_timeOut_));
    m_waitTime_ = m_recordWaitTime_;
}

bool ElapsedTimer::timeOut() noexcept {
    return xTaskCheckForTimeOut(std::addressof(m_timeOut_),
        std::addressof(m_waitTime_));
}

ElapsedTimer::operator bool() noexcept
{ return timeOut(); }

ElapsedTimer::~ElapsedTimer() = default;

#endif
