#ifndef X_ABSTRACT_THREAD_P_HPP
#define X_ABSTRACT_THREAD_P_HPP 1

#include <xabstractthread.hpp>
#include <semaphore.hpp>

#if 0
class XAbstractThreadPrivate final : public XAbstractThreadData {

public:
    W_DECLARE_PUBLIC(XAbstractThread)
    inline static XAtomicInteger<std::size_t> m_th_cnt_{};
    XAtomicInt m_id_{-1};
    XAtomicPointer<void> m_thread_{};
    BinarySemaphore m_sem{};

    explicit XAbstractThreadPrivate() = default;
    ~XAbstractThreadPrivate() override = default;
};

#endif

#endif
