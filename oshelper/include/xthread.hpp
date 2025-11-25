#ifndef X_THREAD_HPP
#define X_THREAD_HPP

#include <xabstractthread.hpp>
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
    //{ setThreadEntry(std::forward<Args>(args)...); }

    XThreadDynamic(XThreadDynamic && ) noexcept;

    XThreadDynamic & operator=(XThreadDynamic && ) noexcept;

    void swap(XThreadDynamic & o) noexcept;

    void start(std::size_t = 1024, uint32_t = configMAX_PRIORITIES) noexcept;
};

#endif

#if configSUPPORT_STATIC_ALLOCATION > 0

template<std::size_t DEPTH>
class XThreadStatic final : public XAbstractThread {
    W_DISABLE_COPY(XThreadStatic)
    static_assert(DEPTH > 0,"DEPTH must be greater than 0");
    std::array<std::size_t, DEPTH> m_stack_{};
    StaticTask_t m_tcb_{};

public:
    XThreadStatic() = default;

    constexpr ~XThreadStatic() override = default;

    template<typename ...Args>
    explicit XThreadStatic(Args && ...args) noexcept
    : XAbstractThread { XCallableHelper::createCallable(std::forward<Args>(args)...) }
    {}
    //{ create_(DEPTH,m_stack_.data(),std::addressof(m_tcb_),std::forward<Args_>(args)...); }

    constexpr XThreadStatic(XThreadStatic && o) noexcept
    { swap(o); }

    constexpr XThreadStatic & operator=(XThreadStatic && o) noexcept
    { XThreadStatic{std::move(o)}.swap(*this); return *this; }

    constexpr void swap(XAbstractThread & o) noexcept {
        m_d_ptr_.swap(o.m_d_ptr_);
        m_stack_.swap(static_cast<XThreadStatic &>(o).m_stack_);
        std::swap(m_tcb_,static_cast<XThreadStatic &>(o).m_tcb_);
    }
};

#endif
#endif
#endif
