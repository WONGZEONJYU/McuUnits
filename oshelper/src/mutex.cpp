#include <mutex.hpp>
#include <mutex>

Mutex::Mutex()
:m_mtxHandle_{ xSemaphoreCreateMutexStatic(&m_semaphore_) }
{}

Mutex::~Mutex()
{ vSemaphoreDelete(m_mtxHandle_); }

void Mutex::lock() const noexcept
{ xSemaphoreTake(m_mtxHandle_,portMAX_DELAY); }

bool Mutex::tryLock(int64_t const wait) const noexcept
{ return pdTRUE == xSemaphoreTake(m_mtxHandle_,wait <= 0 ? portMAX_DELAY : wait); }

void Mutex::unlock() const noexcept
{ xSemaphoreGive(m_mtxHandle_); }
