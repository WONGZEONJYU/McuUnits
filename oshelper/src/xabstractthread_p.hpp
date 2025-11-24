#ifndef X_ABSTRACT_THREAD_P_HPP
#define X_ABSTRACT_THREAD_P_HPP 1

#include <xabstractthread.hpp>
#include <conditionvariable.hpp>
#include <mutex.hpp>

#if 1
class XAbstractThreadPrivate : public XAbstractThreadData {

public:
    W_DECLARE_PUBLIC(XAbstractThread)

    using CallablePtr = XCallableHelper::CallablePtr;

    inline static XAtomicInteger<std::size_t> m_th_cnt{};

    XAtomicInt m_id{-1};
    XAtomicPointer<void> m_thread{};
    Mutex m_mtx{};
    ConditionVariableAny m_cv{};
    XAtomicBool m_finished{true},m_detached{};
    CallablePtr m_fn{};

    static void start(void *);
    explicit XAbstractThreadPrivate() = default;
    ~XAbstractThreadPrivate() override = default;
    void startHelper(std::size_t) noexcept;
    void finished() noexcept;
};

#endif
#endif
