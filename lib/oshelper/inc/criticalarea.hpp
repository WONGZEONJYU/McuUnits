#ifndef CRITICAL_AREA_HPP
#define CRITICAL_AREA_HPP 1

#include <xclasshelpermacros.hpp>
#include <mutex>

#if defined(FREERTOS) || defined(USE_FREERTOS)

class TaskCriticalArea final {
    W_DISABLE_COPY(TaskCriticalArea)
    mutable volatile uint32_t m_count_{};

public:
    explicit TaskCriticalArea() noexcept;
    explicit TaskCriticalArea(std::defer_lock_t);
    ~TaskCriticalArea();
    TaskCriticalArea(TaskCriticalArea &&) noexcept;
    TaskCriticalArea &operator=(TaskCriticalArea &&) noexcept;
    void enter() const noexcept;
    void exit() const noexcept;

private:
    void swap(const TaskCriticalArea & ) const noexcept;
};

class ISRCriticalArea final {
    W_DISABLE_COPY_MOVE(ISRCriticalArea)
    volatile uint32_t m_save_{};
public:
    ISRCriticalArea() noexcept;
    ~ISRCriticalArea();
};

#endif
#endif
