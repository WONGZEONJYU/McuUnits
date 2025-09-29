#include <mutex.hpp>
#include <mutex>

Mutex::Mutex()
#if configSUPPORT_STATIC_ALLOCATION > 0
    :m_mtxHandle_ { xSemaphoreCreateMutexStatic(&m_semaphore_) }
#else
    m_mtxHandle_ { xSemaphoreCreateMutex() }
#endif
{}

Mutex::~Mutex()
{ vSemaphoreDelete(m_mtxHandle_); }

void Mutex::lock() const noexcept
{ xSemaphoreTake(m_mtxHandle_,portMAX_DELAY); }

bool Mutex::tryLock(int64_t const wait) const noexcept
{ return pdTRUE == xSemaphoreTake(m_mtxHandle_,wait <= 0 ? portMAX_DELAY : wait); }

void Mutex::unlock() const noexcept
{ xSemaphoreGive(m_mtxHandle_); }
