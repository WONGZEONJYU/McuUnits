#ifndef X_ABSTRACT_THREAD_P_HPP
#define X_ABSTRACT_THREAD_P_HPP 1

#include <xabstractthread.hpp>
#include <conditionvariable.hpp>
#include <mutex.hpp>

class XAbstractThreadPrivate final : public XAbstractThreadData {

public:
    W_DECLARE_PUBLIC(XAbstractThread)

    using CallablePtr = XCallableHelper::CallablePtr;

    inline static XAtomicInteger<std::size_t> m_th_cnt{};

    XAtomicInt m_id{-1};
    XAtomicPointer<void> m_threadID{};
    XAtomicBool m_finished{true}
        ,m_running{},m_detached{};

    Mutex m_mtx{};
    ConditionVariableAny m_cv{};
    CallablePtr m_fn{};

    static void start(void *) noexcept;
    explicit XAbstractThreadPrivate() = default;
    ~XAbstractThreadPrivate() override = default;
    void startHelper(std::size_t,uint32_t
        ,void * puxStackBuffer,void *pxTaskBuffer ) noexcept;
    void setInfo(void *) noexcept;
    bool waitHelper(int64_t) noexcept;
    void finished() noexcept;
    void setPriority(uint32_t) const noexcept;
};

#endif
