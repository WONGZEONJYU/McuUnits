#include <criticalarea.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#include <mutex>

TaskCriticalArea::TaskCriticalArea() noexcept
{ enter(); }

TaskCriticalArea::TaskCriticalArea(std::defer_lock_t) {}

TaskCriticalArea::~TaskCriticalArea() {
    for (auto i{ m_count_.loadRelaxed() }; i > 0; --i)
    { taskEXIT_CRITICAL(); }
    m_count_.storeRelaxed({});
}

TaskCriticalArea::TaskCriticalArea(TaskCriticalArea && o) noexcept
{ swap(o); }

TaskCriticalArea& TaskCriticalArea::operator=(TaskCriticalArea && o) noexcept
{ TaskCriticalArea { std::move(o) }.swap(*this); return *this; }

void TaskCriticalArea::enter() const noexcept {
    m_count_.fetchAndAddOrdered(1);
    taskENTER_CRITICAL();
}

void TaskCriticalArea::exit() const noexcept {
    if (m_count_.loadAcquire() > 0) {
        m_count_.fetchAndSubOrdered(1);
        taskEXIT_CRITICAL();
    }
}

void TaskCriticalArea::swap(TaskCriticalArea const & o) const noexcept {
    auto const c{ m_count_.loadRelaxed() };
    m_count_.storeRelease(c);
    o.m_count_.storeRelease(c);
}

ISRCriticalArea::ISRCriticalArea() noexcept
:m_save_(taskENTER_CRITICAL_FROM_ISR())
{}

ISRCriticalArea::~ISRCriticalArea()
{ taskEXIT_CRITICAL_FROM_ISR(m_save_); }

#endif
