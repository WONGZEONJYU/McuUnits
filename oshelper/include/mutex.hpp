#ifndef MUTEX_HPP
#define MUTEX_HPP 1

#include <wglobal.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <semphr.h>

class Mutex final {
    W_DISABLE_COPY_MOVE(Mutex)
    mutable SemaphoreHandle_t m_mtxHandle_{};
    mutable StaticSemaphore_t m_semaphore_{};

public:
    explicit Mutex();
    ~Mutex();
    void lock() const noexcept;
    bool tryLock(int64_t = 1) const noexcept;
    void unlock() const noexcept;
};

#endif
#endif
