#include <binarySemaphore.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

BinarySemaphore::BinarySemaphore() {
#if configSUPPORT_STATIC_ALLOCATION > 0
    m_BinarySemaphore = xSemaphoreCreateBinaryStatic(&m_SemaphoreBuffer);
#else
    m_BinarySemaphore = xSemaphoreCreateBinary();
#endif
}

BinarySemaphore::~BinarySemaphore()
{ vSemaphoreDelete(m_BinarySemaphore); }

bool BinarySemaphore::acquire(int64_t const wait) const noexcept
{ return xSemaphoreTake(m_BinarySemaphore,wait < 0 ? portMAX_DELAY : static_cast<TickType_t>(wait)); }

bool BinarySemaphore::acquireFromISR() const noexcept {
    BaseType_t x{},b { xSemaphoreTakeFromISR( m_BinarySemaphore,&x) };
    portYIELD_FROM_ISR(x);
    return static_cast<bool>(b);
}

bool BinarySemaphore::releases() const noexcept
{ return xSemaphoreGive(m_BinarySemaphore); }

bool BinarySemaphore::releasesFromISR() const noexcept {
    BaseType_t x{},b { xSemaphoreGiveFromISR(m_BinarySemaphore,&x) };
    portYIELD_FROM_ISR(x);
    return static_cast<bool>(b);
}

#endif
