#include <criticalarea.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#include <mutex>
#include <atomic.h>

TaskCriticalArea::TaskCriticalArea() noexcept
{ enter(); }

TaskCriticalArea::TaskCriticalArea(std::defer_lock_t) {}

TaskCriticalArea::~TaskCriticalArea() {
    for (auto i{ m_count_ }; i > 0; --i)
    { taskEXIT_CRITICAL(); }
    m_count_ = {};
}

TaskCriticalArea::TaskCriticalArea(TaskCriticalArea && o) noexcept
{ swap(o); }

TaskCriticalArea& TaskCriticalArea::operator=(TaskCriticalArea && o) noexcept
{ TaskCriticalArea { std::move(o) }.swap(*this); return *this; }

void TaskCriticalArea::enter() const noexcept {
    taskENTER_CRITICAL();
    Atomic_Increment_u32(std::addressof(m_count_));
}

void TaskCriticalArea::exit() const noexcept {
    if (m_count_> 0) {
        Atomic_Decrement_u32(std::addressof(m_count_));
        taskEXIT_CRITICAL();
    }
}

void TaskCriticalArea::swap(TaskCriticalArea const & o) const noexcept {
    auto const c{ m_count_ };
    m_count_ = o.m_count_;
    o.m_count_ = c;
}

ISRCriticalArea::ISRCriticalArea() noexcept
:m_save_(taskENTER_CRITICAL_FROM_ISR())
{   }

ISRCriticalArea::~ISRCriticalArea()
{ taskEXIT_CRITICAL_FROM_ISR(m_save_); }

#endif
