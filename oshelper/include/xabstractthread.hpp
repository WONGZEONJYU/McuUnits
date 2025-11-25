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
#if 0
protected:
    inline static XAtomicInteger<std::size_t> m_th_cnt_{};
    XAtomicInt m_id_{-1};
    XAtomicPointer<void> m_thread_{};
#endif
    W_DISABLE_COPY(XAbstractThread)
    W_DECLARE_PRIVATE_D(m_d_ptr_,XAbstractThread)
protected:
    XUniquePtr<XAbstractThreadData> m_d_ptr_{};

public:
    static std::size_t thread_count() noexcept;
    [[nodiscard]] void * thread_handle() const noexcept;
    [[nodiscard]] int thread_id() const noexcept;

#if 0
    void start() const noexcept;
    void stop() const noexcept;
#endif
    bool wait(int64_t = -1) noexcept;
    void setPriority(uint32_t) const noexcept;

#if 0
    virtual void swap(XAbstractThread & ) noexcept;
#endif
    template<typename ...Args> void setThreadEntry(Args && ...args) noexcept
    { setThreadFn(XCallableHelper::createCallable(std::forward<Args>(args)...)); }

    static void sleep_for(std::size_t) noexcept;
    static void sleep_until(std::size_t & ,std::size_t) noexcept;
    static void yield() noexcept;
    static bool isRunningInThread() noexcept;
    virtual ~XAbstractThread();

protected:
    using CallablePtr = XCallableHelper::CallablePtr;
    void destroy() noexcept;
    void start(std::size_t ,uint32_t ,void * ,void *) noexcept;

private:
    XAbstractThread() ;
    explicit XAbstractThread(CallablePtr);
    void setThreadFn(CallablePtr);

#if 0
    template <typename , size_t... > static constexpr void Invoke_(void * ) noexcept;
    template <typename , size_t... Indices_> static constexpr auto GetInvoke_(std::index_sequence<Indices_...>) noexcept;
    template<typename ...Args_> constexpr void create_(std::size_t ,void * ,void * ,Args_ && ...) noexcept;
    static void taskReturn() noexcept;
    void createTask(void(*f)(void*),std::size_t uxStackDepth,void * pvParameters,void * = {},void * = {}) noexcept;
    constexpr XAbstractThread() = default;
    void setInfo(void *) noexcept;
#endif
    friend class XThreadDynamic;
    template<std::size_t > friend class XThreadStatic;
};

#if 0
template <typename Tuple_, size_t... Indices_>
constexpr void XAbstractThread::Invoke_(void * const RawVals_) noexcept {
    XUniquePtr<Tuple_> const FnVals_(static_cast<Tuple_*>(RawVals_));
    auto & Tup_ {*FnVals_};
    std::invoke(std::get<Indices_>(std::forward<Tuple_>(Tup_))...);
    taskReturn();
}

template <typename Tuple_, size_t... Indices_>
constexpr auto XAbstractThread::GetInvoke_(std::index_sequence<Indices_...>) noexcept
{ return &Invoke_<Tuple_, Indices_...>; }

template<typename ...Args_>
constexpr void XAbstractThread::create_(std::size_t const stack_depth,void * const puxStackBuffer
    ,void * const pxTaskBuffer,Args_ && ...args) noexcept
{
    using Tuple_ = std::tuple<std::decay_t<Args_>...>;
    auto Decay_copied_ { makeUnique<Tuple_>(std::forward<Args_>(args)...) };
    constexpr auto Invoker_proc { GetInvoke_<Tuple_>(std::make_index_sequence<sizeof...(Args_)>{}) };
    createTask(Invoker_proc,stack_depth,Decay_copied_.release(),puxStackBuffer,pxTaskBuffer);
}
#endif

#endif
