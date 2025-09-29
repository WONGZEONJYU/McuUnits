#ifndef X_THREAD_HPP
#define X_THREAD_HPP

#include <xthreadbase.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>

#if configSUPPORT_DYNAMIC_ALLOCATION > 0

class XThreadDynamic final : public XThreadBase {
    W_DISABLE_COPY(XThreadDynamic)

public:
    XThreadDynamic() = default;

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
class XThreadStatic final : public XThreadBase {
    W_DISABLE_COPY(XThreadStatic)
    static_assert(DEPTH > 0,"DEPTH must be greater than 0");
    std::array<std::size_t, DEPTH> m_stack_{};
    StaticTask_t m_tcb_{};

public:
    XThreadStatic() = default;

    ~XThreadStatic() override = default;

    template<typename ...Args_>
    explicit XThreadStatic(Args_ && ...args) noexcept
    { create_(DEPTH,m_stack_.data(),std::addressof(m_tcb_),std::forward<Args_>(args)...); }

    XThreadStatic(XThreadStatic && o) noexcept
    { swap(o); }

    XThreadStatic & operator=(XThreadStatic && o) noexcept
    { XThreadStatic{std::move(o)}.swap(*this); return *this; }

private:
    void swap(XThreadBase & o) noexcept override {
        XThreadBase::swap(o);
        m_stack_.swap(static_cast<XThreadStatic &>(o).m_stack_);
        std::swap(m_tcb_,static_cast<XThreadStatic &>(o).m_tcb_);
    }
};

#endif
#endif
#endif
