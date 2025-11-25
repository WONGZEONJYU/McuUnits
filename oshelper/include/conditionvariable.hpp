#ifndef CONDITION_VARIABLE_HPP
#define CONDITION_VARIABLE_HPP 1

#include <mutex>
#include <wglobal.hpp>
#include <xatomic.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <semaphore.hpp>

class ConditionVariableAny final {
    W_DISABLE_COPY(ConditionVariableAny)

    template<typename Lock_> struct UnLockGuard final {
        W_DISABLE_COPY(UnLockGuard)
        Lock_ & m_lock_{};
        explicit constexpr UnLockGuard(Lock_ & lock) noexcept: m_lock_(lock)
        { lock.unlock(); }
        constexpr ~UnLockGuard() noexcept { m_lock_.lock(); }
    };

    template<typename ATOMIC> struct WaiterGuard final {
        ATOMIC & m_cnt_{};
        W_DISABLE_COPY(WaiterGuard)
        constexpr explicit WaiterGuard(ATOMIC & a) noexcept : m_cnt_(a)
        { a.ref(); }
        constexpr ~WaiterGuard() noexcept
        { m_cnt_.deref(); }
    };

#if 0
#if configSUPPORT_STATIC_ALLOCATION > 0
    mutable StaticSemaphore_t m_semaphore_{};
#endif
    mutable SemaphoreHandle_t m_semaphoreHandle_{};
#else
    mutable CountingSemaphore<> m_semaphore_{};
#endif
    mutable XAtomicInteger<std::size_t> m_waiters_{};

public:
    explicit ConditionVariableAny();

    ~ConditionVariableAny();

    // wait 解锁 -> 阻塞 -> 重新加锁
    template<typename Lock>
    constexpr void wait(Lock & lock) noexcept
    { waitImpl(lock); }

    template<typename Lock>
    constexpr bool wait_for(Lock & lock, uint32_t const timeoutMs) noexcept
    { return waitImpl(lock, pdMS_TO_TICKS(timeoutMs)); }

    // Predicate 版本
    template<typename Lock, typename Predicate>
    constexpr void wait(Lock & lock, Predicate pred) noexcept
    { while (!pred()) { wait(lock); } }

    template<typename Lock, typename Predicate>
    constexpr bool wait_for(Lock & lock, uint32_t timeoutMs, Predicate pred) noexcept;

    void notify_one() const noexcept;

    void notify_all() const noexcept;

private:
    template<typename Lock>
    constexpr bool waitImpl(Lock & lock, int64_t const ticks = -1) noexcept
    { UnLockGuard unlock { lock }; WaiterGuard w {m_waiters_}; return m_semaphore_.acquire(ticks); }
};

template<typename Lock, typename Predicate>
constexpr bool ConditionVariableAny::wait_for(Lock & lock, uint32_t const timeoutMs, Predicate pred) noexcept {
    if (pred() ) { return true; }
    if (!timeoutMs) { return pred(); }

    auto waitTime { pdMS_TO_TICKS(timeoutMs) };
    TimeOut_t timeout{};
    vTaskSetTimeOutState( &timeout );

    while (!pred()) {
        if (xTaskCheckForTimeOut(std::addressof(timeout),std::addressof(waitTime)))
        { return pred(); }
        if ( !waitImpl(lock, waitTime) ) { return pred(); }
    }
    return true;
}

#endif
#endif
