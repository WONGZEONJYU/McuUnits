#include <timeout.hpp>
#include <xmemory.hpp>
#include <cassert>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>

class ElapsedTimerPrivate final {
public:
    ElapsedTimer * q_ptr{};
    TimeOut_t m_timeOut{};
    TickType_t m_waitTime{},m_recordWaitTime{};
    W_DECLARE_PUBLIC(ElapsedTimer)
    explicit ElapsedTimerPrivate(ElapsedTimer * const q):q_ptr{ q }
    {   }
    ~ElapsedTimerPrivate() = default;
};

ElapsedTimer::ElapsedTimer(std::chrono::milliseconds const ms) noexcept :
    m_d_ptr_{ makeUnique<ElapsedTimerPrivate>(this) }
{
    assert(m_d_ptr_);
    setTimeOut(ms);
    vTaskSetTimeOutState(std::addressof(d_func()->m_timeOut));
}

ElapsedTimer::ElapsedTimer(uint32_t const ms) noexcept:
    ElapsedTimer { std::chrono::milliseconds{ms} }
{   }

void ElapsedTimer::setTimeOut(uint32_t const ms) noexcept {
    W_D(ElapsedTimer);
    d->m_waitTime = pdMS_TO_TICKS(ms);
    d->m_recordWaitTime = d->m_waitTime;
}

void ElapsedTimer::setTimeOut(std::chrono::milliseconds const ms) noexcept
{ setTimeOut(ms.count()); }

void ElapsedTimer::restart() noexcept {
    vTaskSetTimeOutState(std::addressof(d_func()->m_timeOut));
    W_D(ElapsedTimer);
    d->m_waitTime = d->m_recordWaitTime;
}

bool ElapsedTimer::timeOut() noexcept {
    W_D(ElapsedTimer);
    return xTaskCheckForTimeOut(std::addressof(d->m_timeOut),
        std::addressof(d->m_waitTime));
}

ElapsedTimer::operator bool() noexcept
{ return timeOut(); }

ElapsedTimer::~ElapsedTimer() = default;

#endif
