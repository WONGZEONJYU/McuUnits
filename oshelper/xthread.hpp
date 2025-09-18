#ifndef XTHREAD_HPP
#define XTHREAD_HPP

#include <FreeRTOS.h>
#include <task.h>
#include <xhelper.hpp>
#include <sstream>

#if defined(FREERTOS) || defined(USE_FREERTOS)

class Task_CRITICAL final {
    X_DISABLE_COPY_MOVE(Task_CRITICAL)
    std::atomic_ulong m_cnt_{};

public:
    explicit Task_CRITICAL() noexcept
    { enter(); }

    ~Task_CRITICAL() noexcept
    { exit(); }

    void enter() noexcept
    { ++m_cnt_; taskENTER_CRITICAL(); }

    void exit() noexcept
    { --m_cnt_; taskEXIT_CRITICAL(); }
};

template<uint32_t > class XThread_;

template<uint32_t STACK_DEPTH = {}>
class XThread_ final {

    X_DISABLE_COPY(XThread_)

    struct [[maybe_unused]] Static_Task_Data {
        std::array<uint32_t, STACK_DEPTH> m_stack{};
        StaticTask_t m_tcb{};
    };

    void destroy() noexcept {
        if (m_thread_) {
            vTaskSuspend(m_thread_);
            vTaskDelete(m_thread_);
            m_thread_ = {};
        }
    }

    template <typename Tuple_, size_t... Indices_>
    static void Invoke_(void* RawVals_) noexcept {
        const std::unique_ptr<Tuple_> FnVals_(static_cast<Tuple_*>(RawVals_));
        Tuple_& Tup_ {*FnVals_.get()};
        std::invoke(std::move(std::get<Indices_>(Tup_))...);
        vTaskDelete(nullptr);
    }

    template <typename Tuple_, size_t... Indices_>
    [[nodiscard]] static constexpr auto Get_invoke_(std::index_sequence<Indices_...>) noexcept {
        return &Invoke_<Tuple_, Indices_...>;
    }

    template<typename Fn_,typename ...Args_>
    void Start_(uint32_t stack_depth,Fn_ &&fn,Args_ &&...args) noexcept {
        static std::atomic_int thread_index{};
        std::stringstream ss;
        ss << GET_STR(thread:) << thread_index;

        using Tuple_ = std::tuple<std::decay_t<Fn_>, std::decay_t<Args_>...>;
        auto Decay_copied_{std::make_unique<Tuple_>(std::forward<Fn_>(fn),std::forward<Args_>(args)...)};
        constexpr auto Invoker_proc { Get_invoke_<Tuple_>(std::make_index_sequence<1 + sizeof...(Args_)>{}) };

        tskTaskControlBlock* th_{};
        Task_CRITICAL c{}; /*进入临界区*/
        if (STACK_DEPTH > 0) {
#if configSUPPORT_STATIC_ALLOCATION > 0 && !(configSUPPORT_DYNAMIC_ALLOCATION > 0)
            stack_depth = STACK_DEPTH;
            StaticTask_t tcb_{};
            th_ = xTaskCreateStatic(Invoker_proc,ss.str().c_str(),
                stack_depth,Decay_copied_.get(),
                configMAX_PRIORITIES,m_std_.load().m_stack,&tcb_);
            m_std_.load().m_tcb = tcb_;
#endif
        }else {
#if configSUPPORT_DYNAMIC_ALLOCATION > 0
            (void)xTaskCreate(Invoker_proc,ss.str().c_str(),
                stack_depth,Decay_copied_.get(),configMAX_PRIORITIES,&th_);
#endif
        }

        if (th_) {
            m_thread_ = th_;
            m_id = thread_index.load();
            m_th_cnt_ = ++thread_index;
            Decay_copied_.release();
            vTaskSuspend(th_);
        }
    }

public:
    [[nodiscard]] auto thread_handle() const noexcept
    { return m_thread_.load(); }

    static auto thread_count() noexcept
    { return m_th_cnt_.load(); }

    [[nodiscard]] auto thread_id() const noexcept
    { return m_id.load(); }

    void Start() const noexcept
    { if (m_thread_) { vTaskResume(m_thread_); } }

    void Stop() const noexcept
    { if (m_thread_) { vTaskSuspend(m_thread_); } }

    void Set_Priority(const uint32_t &p) const noexcept
    { if (m_thread_){ vTaskPrioritySet(m_thread_,p); } }

    template<typename Fn_,typename ...Args_>
    explicit XThread_(const uint32_t &stack_depth,Fn_ &&fn,Args_&& ...args)
    { Start_(stack_depth,std::forward<Fn_>(fn),std::forward<Args_>(args)...); }

    XThread_(XThread_ &&obj) noexcept {
        m_thread_ = obj.m_thread_.exchange({});
#if configSUPPORT_STATIC_ALLOCATION > 0
        if(STACK_DEPTH > 0) {
            m_std_ = obj.m_std_.exchange({});
        }
#endif
    }

    XThread_& operator=(XThread_ &&obj) noexcept {
        if (this != &obj) {
            destroy();
            m_thread_ = obj.m_thread_.exchange({});
#if configSUPPORT_STATIC_ALLOCATION > 0
            if(STACK_DEPTH > 0) {
                m_std_ = obj.m_std_.exchange({});
            }
#endif
        }
        return *this;
    }

    ~XThread_()
    { destroy(); }

    XThread_() = default;

private:
    inline static std::atomic_uint32_t m_th_cnt_{};
    std::atomic_int m_id{-1};
    std::atomic<tskTaskControlBlock *> m_thread_{};
#if configSUPPORT_STATIC_ALLOCATION > 0
    std::atomic<Static_Task_Data> m_std_{};
#endif
};

using XThread_Dynamic = XThread_<>;

template<uint32_t STACK_DEPTH = 1024>
using XThread_Static = XThread_<STACK_DEPTH>;

#endif
#endif
