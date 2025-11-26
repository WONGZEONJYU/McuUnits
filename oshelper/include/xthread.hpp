#ifndef X_THREAD_HPP
#define X_THREAD_HPP

#include <xabstractthread.hpp>
#include <xcontainer.hpp>
#include <criticalarea.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>

#if configSUPPORT_DYNAMIC_ALLOCATION > 0

class XThreadDynamic final : public XAbstractThread {

    W_DISABLE_COPY(XThreadDynamic)

public:
    XThreadDynamic() = default;

    ~XThreadDynamic() override;

    template<typename ...Args>
    explicit XThreadDynamic(Args && ...args)
    : XAbstractThread { XCallableHelper::createCallable(std::forward<Args>(args)...) }
    {}

    XThreadDynamic(XThreadDynamic && ) noexcept;

    XThreadDynamic & operator=(XThreadDynamic && ) noexcept;

    void swap(XThreadDynamic & o) noexcept;

    void start(std::size_t = 1024, uint32_t = configMAX_PRIORITIES - 1) noexcept;
};

#endif

#if configSUPPORT_STATIC_ALLOCATION > 0

template<std::size_t = 1024> class XThreadStatic;

template<std::size_t DEPTH>
class XThreadStatic final : public XAbstractThread {

    W_DISABLE_COPY(XThreadStatic)
    static_assert(DEPTH > 0,"DEPTH must be greater than 0");
    std::array<StackType_t, DEPTH> m_stack_{};
    XUniquePtr<StaticTask_t> m_tcb_{};

public:
    constexpr XThreadStatic() = default;

    constexpr ~XThreadStatic() override
    { wait(); }

    template<typename ...Args>
    explicit XThreadStatic(Args && ...args) noexcept
    : XAbstractThread { XCallableHelper::createCallable(std::forward<Args>(args)...) }
    {}

    constexpr XThreadStatic(XThreadStatic && o) noexcept { swap(o); }

    constexpr XThreadStatic & operator=(XThreadStatic && o) noexcept
    { if (this != std::addressof(o)) { wait(); swap(o); } return *this; }

    constexpr void swap(XThreadStatic & o) noexcept {
        std::swap(m_d_ptr_, o.m_d_ptr_);
        std::swap(m_stack_,o.m_stack_);
        std::swap(m_tcb_, o.m_tcb_);
    }

    constexpr void start(uint32_t const prio = configMAX_PRIORITIES - 1) noexcept {
        if (!isRunningInThread() || isRunning() || !isFinished() ) { return; }
        auto tcb{ makeUnique<StaticTask_t>() };
        if (!tcb) { return; }
        m_tcb_.swap(tcb);
        XAbstractThread::start(m_stack_.size(), prio,m_stack_.data(),m_tcb_.get());
    }
};

#endif
#endif
#endif
