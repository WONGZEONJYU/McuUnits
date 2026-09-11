#include <timeout.hpp>
#include <xmemory.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>

class ElapsedTimerPrivate final {
public:
    ElapsedTimer * q_ptr{};
    TimeOut_t m_timeOut{};
    TickType_t m_waitTime{};
    W_DECLARE_PUBLIC(ElapsedTimer)
    explicit ElapsedTimerPrivate(ElapsedTimer * const q):q_ptr{ q }
    {   }
    ~ElapsedTimerPrivate() = default;
};

ElapsedTimer::ElapsedTimer(std::chrono::milliseconds const ms) noexcept :
    m_d_ptr_{ makeUnique<ElapsedTimerPrivate>(this) }
{ setTimeOut(ms); }

ElapsedTimer::ElapsedTimer(uint32_t const ms) noexcept:
    ElapsedTimer { std::chrono::milliseconds{ms} }
{   }

void ElapsedTimer::setTimeOut(uint32_t const ms) noexcept
{ d_func()->m_waitTime = pdMS_TO_TICKS(ms); }

void ElapsedTimer::setTimeOut(std::chrono::milliseconds const ms) noexcept
{ d_func()->m_waitTime = pdMS_TO_TICKS(ms.count()); }

void ElapsedTimer::reload() noexcept
{ vTaskSetTimeOutState(std::addressof(d_func()->m_timeOut)); }

bool ElapsedTimer::timeOut() noexcept {
    W_D(ElapsedTimer);
    return xTaskCheckForTimeOut(std::addressof(d->m_timeOut),
        std::addressof(d->m_waitTime));
}

ElapsedTimer::operator bool() noexcept
{ return timeOut(); }

ElapsedTimer::~ElapsedTimer() = default;

#endif
