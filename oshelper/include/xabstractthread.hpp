#ifndef X_THREAD_P_HPP
#define X_THREAD_P_HPP 1

#include <xmemory.hpp>
#include <xcallablehelper.hpp>

#if 1
class XAbstractThread;
class XAbstractThreadPrivate;

class XAbstractThreadData {
protected:
    constexpr XAbstractThreadData() = default;
public:
    virtual ~XAbstractThreadData() = default;
    XAbstractThread * q_ptr{};
};
#endif

class XAbstractThread {

    W_DISABLE_COPY(XAbstractThread)
    W_DECLARE_PRIVATE_D(m_d_ptr_,XAbstractThread)
    XUniquePtr<XAbstractThreadData> m_d_ptr_{};

public:
    static std::size_t threadCount() noexcept;
    static void sleep_for(std::size_t) noexcept;
    static void sleep_until(std::size_t & ,std::size_t) noexcept;
    static void yield() noexcept;
    static bool isRunningInThread() noexcept;

    [[nodiscard]] void * threadHandle() const noexcept;
    /**
     * 自定义ID,紧记录当前线程编号,不能当句柄使用
     * @return
     */
    [[nodiscard]] int threadID() const noexcept;

    bool wait(int64_t = -1) noexcept;
    /**
     * 如果调用了detach(),wait不再有效,永远返回true
     */
    void detach() noexcept;
    [[nodiscard]] bool isFinished() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    void setPriority(uint32_t) const noexcept;

    template<typename ...Args> void setThreadEntry(Args && ...args) noexcept
    { setThreadFn(XCallableHelper::createCallable(std::forward<Args>(args)...)); }

    virtual ~XAbstractThread();

private:
    using CallablePtr = XCallableHelper::CallablePtr;
    XAbstractThread();
    explicit XAbstractThread(CallablePtr);
    void destroy() noexcept;
    void setThreadFn(CallablePtr);
    void start(std::size_t ,uint32_t ,void * ,void *) noexcept;

    friend class XThreadDynamic;
    template<std::size_t > friend class XThreadStatic;
};

#endif
