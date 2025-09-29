#ifndef CRITICAL_AREA_HPP
#define CRITICAL_AREA_HPP 1

#include <wglobal.hpp>
#include <mutex>
#include <xatomic.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

class TaskCriticalArea final {
    W_DISABLE_COPY(TaskCriticalArea)
    mutable XAtomicInteger<int_fast32_t> m_count_{};

public:
    explicit TaskCriticalArea() noexcept;
    explicit TaskCriticalArea(std::defer_lock_t);
    ~TaskCriticalArea();
    TaskCriticalArea(TaskCriticalArea &&) noexcept;
    TaskCriticalArea &operator=(TaskCriticalArea &&) noexcept;
    void enter() const noexcept;
    void exit() const noexcept;

private:
    void swap(TaskCriticalArea const & ) const noexcept;
};

class ISRCriticalArea final {
    W_DISABLE_COPY_MOVE(ISRCriticalArea)
    XAtomicInteger<uint32_t> m_save_{};
public:
    ISRCriticalArea() noexcept;
    ~ISRCriticalArea();
};

#endif
#endif
