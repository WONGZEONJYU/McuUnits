#include <criticalarea.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#include <mutex>

constexpr TaskCriticalArea::TaskCriticalArea() noexcept
{ enter(); }

constexpr TaskCriticalArea::TaskCriticalArea(std::defer_lock_t) noexcept {}

constexpr TaskCriticalArea::~TaskCriticalArea() {
    for (auto i{ m_count_.load(std::memory_order_relaxed) }; i > 0; --i)
    { taskEXIT_CRITICAL(); }
    m_count_.store(0, std::memory_order_relaxed);
}

TaskCriticalArea::TaskCriticalArea(TaskCriticalArea && o) noexcept
{ swap(o); }

TaskCriticalArea& TaskCriticalArea::operator=(TaskCriticalArea && o) noexcept
{ TaskCriticalArea { std::move(o) }.swap(*this); return *this; }

void TaskCriticalArea::enter() const noexcept {
    m_count_.fetch_add(1, std::memory_order_relaxed);
    taskENTER_CRITICAL();
}

void TaskCriticalArea::exit() const noexcept {
    if (m_count_.load(std::memory_order_relaxed) > 0) {
        m_count_.fetch_sub(1, std::memory_order_relaxed);
        taskEXIT_CRITICAL();
    }
}

void TaskCriticalArea::swap(TaskCriticalArea const & o) const noexcept
{ m_count_.exchange(o.m_count_.load(std::memory_order_relaxed),std::memory_order_relaxed); }

constexpr ISRCriticalArea::ISRCriticalArea()
:m_save_(taskENTER_CRITICAL_FROM_ISR())
{}

constexpr ISRCriticalArea::~ISRCriticalArea()
{ taskEXIT_CRITICAL_FROM_ISR(m_save_); }

#endif
