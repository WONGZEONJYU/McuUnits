#ifndef X_THREAD_HPP
#define X_THREAD_HPP

#include <xabstractthread.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>

#if configSUPPORT_DYNAMIC_ALLOCATION > 0

class XThreadDynamic final : public XAbstractThread {
    W_DISABLE_COPY(XThreadDynamic)

public:
    constexpr XThreadDynamic() = default;

    ~XThreadDynamic() override = default;

    template<typename ...Args_>
    explicit XThreadDynamic(std::size_t const stack_depth,Args_ && ...args) noexcept
    { create_(stack_depth,{},{},std::forward<Args_>(args)...); }

    XThreadDynamic(XThreadDynamic && ) noexcept;

    XThreadDynamic & operator=(XThreadDynamic && ) noexcept;
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
    constexpr XThreadStatic() = default;

    constexpr ~XThreadStatic() override = default;

    template<typename ...Args_>
    constexpr explicit XThreadStatic(Args_ && ...args) noexcept
    { create_(DEPTH,m_stack_.data(),std::addressof(m_tcb_),std::forward<Args_>(args)...); }

    constexpr XThreadStatic(XThreadStatic && o) noexcept
    { swap(o); }

    constexpr XThreadStatic & operator=(XThreadStatic && o) noexcept
    { XThreadStatic{std::move(o)}.swap(*this); return *this; }

    constexpr void swap(XAbstractThread & o) noexcept override {
        XAbstractThread::swap(o);
        m_stack_.swap(static_cast<XThreadStatic &>(o).m_stack_);
        std::swap(m_tcb_,static_cast<XThreadStatic &>(o).m_tcb_);
    }
};

#endif
#endif
#endif
