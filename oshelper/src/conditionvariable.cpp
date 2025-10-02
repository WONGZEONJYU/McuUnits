#include <conditionvariable.hpp>

#if 0
ConditionVariableAny::ConditionVariableAny()
#if configSUPPORT_STATIC_ALLOCATION > 0
:m_semaphoreHandle_{ xSemaphoreCreateCountingStatic(configMAX_PRIORITIES,0,&m_semaphore_) }
#else
:m_semaphoreHandle_ { xSemaphoreCreateCounting(configMAX_PRIORITIES, 0); }
#endif
{}

ConditionVariableAny::~ConditionVariableAny()
{ if (m_semaphoreHandle_) { vSemaphoreDelete(m_semaphoreHandle_);} }

void ConditionVariableAny::notify_one() const noexcept
{ if (m_semaphoreHandle_) { xSemaphoreGive(m_semaphoreHandle_); } }

void ConditionVariableAny::notify_all(size_t const count) const noexcept {
    if (!m_semaphoreHandle_) { return; }
    for (size_t i {}; i < count; ++i) {
        xSemaphoreGive(m_semaphoreHandle_);
    }
}

#else

ConditionVariableAny::ConditionVariableAny() = default;

ConditionVariableAny::~ConditionVariableAny() = default;

void ConditionVariableAny::notify_one() const noexcept
{ (void)m_semaphore_.releases(); }

void ConditionVariableAny::notify_all() const noexcept {
    auto const n { m_waiters_.loadAcquire() };
    for (size_t i {}; i < n; ++i)
    { (void)m_semaphore_.releases(); }
}

#endif
