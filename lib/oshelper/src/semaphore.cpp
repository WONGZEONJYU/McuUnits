#include <semaphore.hpp>
#include <memory>
#if defined(FREERTOS) || defined(USE_FREERTOS)

BinarySemaphore::BinarySemaphore()
#if configSUPPORT_STATIC_ALLOCATION > 0
:m_semaphoreHandle_ { xSemaphoreCreateBinaryStatic(std::addressof(m_semaphore_)) }
#else
:m_semaphoreHandle_ { xSemaphoreCreateBinary() }
#endif
{}

BinarySemaphore::~BinarySemaphore()
{ vSemaphoreDelete(m_semaphoreHandle_); }

bool BinarySemaphore::acquire(int64_t const wait) const noexcept
{ return xSemaphoreTake(m_semaphoreHandle_,wait < 0 ? portMAX_DELAY : static_cast<TickType_t>(wait)); }

bool BinarySemaphore::acquireFromISR() const noexcept {
    BaseType_t x{},b { xSemaphoreTakeFromISR( m_semaphoreHandle_,&x) };
    portYIELD_FROM_ISR(x);
    return static_cast<bool>(b);
}

bool BinarySemaphore::releases() const noexcept
{ return xSemaphoreGive(m_semaphoreHandle_); }

bool BinarySemaphore::releasesFromISR() const noexcept {
    BaseType_t x{},b { xSemaphoreGiveFromISR(m_semaphoreHandle_,&x) };
    portYIELD_FROM_ISR(x);
    return static_cast<bool>(b);
}

#endif
