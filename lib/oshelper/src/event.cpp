#include <event.hpp>
#include <cassert>
#include <mutex.hpp>
#include <xcontainer.hpp>
#include <timeout.hpp>
#include <xatomic.hpp>
#include <xabstractthread.hpp>
#include <xscopeguard.hpp>
#include <criticalarea.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

namespace {

    using timeEvent_t = std::pair<ElapsedTimer, event_fn>;

    class EventDispatcher final {
        Mutex mutable m_events_mtx_{},m_timeEvents_mtx_{};
        XDeque<event_fn> m_events_{};
        XList<timeEvent_t> m_delayEvents_{};
        XAtomicInt m_ref_{};
        void * m_ownershipThread_ { xTaskGetCurrentTaskHandle() };

        struct RefGuard final {
            explicit RefGuard(EventDispatcher * const obj) noexcept :m_obj_{obj}
            { obj->ref();}
            ~RefGuard() noexcept { m_obj_->deref(); }
        private:
            EventDispatcher * m_obj_{};
        };

    public:
        explicit EventDispatcher() = default;
        ~EventDispatcher() = default;

        constexpr void * ownershipThread() const noexcept
        { return m_ownershipThread_; }

        void ref() noexcept
        { m_ref_.ref(); }

        bool deref() noexcept
        { return m_ref_.deref(); }

        void postEvent(event_fn fn) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_events_mtx_};
            m_events_.push_back(std::move(fn));
        }

        void postTimeEvent(event_fn fn,uint32_t ms) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_timeEvents_mtx_};
            m_delayEvents_.emplace_back(ms,std::move(fn));
        }

        void postTimeEvent(event_fn fn,std::chrono::milliseconds ms) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_timeEvents_mtx_};
            m_delayEvents_.emplace_back(ms,std::move(fn));
        }

        void processEvent() noexcept {

            processTimeEvent();

            auto const get {
                [this]() noexcept -> event_fn {
                    std::unique_lock lk{m_events_mtx_};
                    if (m_events_.empty()) { return {}; }
                    auto ret{ std::move(m_events_.front()) };
                    m_events_.pop_front();
                    return ret;
                }
            };

            while (true) {
                auto const event{ get() };
                if (!event) { return; }
                event();
            }

            XAbstractThread::yield();
        }

    private:
        void processTimeEvent() noexcept {
            std::unique_lock lk {m_timeEvents_mtx_};
            for (auto it { m_delayEvents_.begin() };it != m_delayEvents_.end();) {
                auto && evt { *it };
                auto && [t,e] { evt };
                t ? m_events_.push_back(std::move(e)),it = m_delayEvents_.erase(it) : ++it;
            }
        }
    };

    class EventDispatcherTls final {

        static void setCurrent(void * const v) noexcept
        { vTaskSetThreadLocalStoragePointer({},{},v); }

    public:
        static auto getCurrent() noexcept
        { return static_cast<EventDispatcher*>(pvTaskGetThreadLocalStoragePointer({},{})); }

        static EventDispatcher * current() noexcept {
            TaskCriticalArea t{};
            auto h{ getCurrent() };
            if (!h) {
                h = makeUnique<EventDispatcher>().release();
                if (h) { setCurrent(h); }
            }
            return h;
        }

        static void remove(EventDispatcher * const ev) noexcept {
            TaskCriticalArea t{};
            if (!ev || ev->ownershipThread() != xTaskGetCurrentTaskHandle() || ev != getCurrent() || ev->deref() ) { return; }
            setCurrent({});
            XUniquePtr<EventDispatcher> { ev };
        }
    };

}

class EventLoopPrivate final {

public:
    EventLoop * q_ptr{};
    EventDispatcher * m_eventDispatcher{};
    bool m_inExec_{};
    XAtomicInt m_exits_{};
    W_DECLARE_PUBLIC(EventLoop)
    EventLoopPrivate() = default;
    ~EventLoopPrivate() = default;
};

EventLoop::EventLoop()
    :m_d_ptr_{ makeUnique<EventLoopPrivate>() }
{
    configASSERT(m_d_ptr_);
    m_d_ptr_->q_ptr = this;
    W_D(EventLoop);
    auto const evt{ EventDispatcherTls::current() };
    configASSERT(evt);
    evt->ref();
    d->m_eventDispatcher = evt;
}

EventLoop::~EventLoop()
{ EventDispatcherTls::remove(d_func()->m_eventDispatcher); }

void EventLoop::exec() noexcept {
    W_D(EventLoop);

    if (d->m_inExec_) { return; }

    configASSERT(xTaskGetCurrentTaskHandle() == d->m_eventDispatcher->ownershipThread());

    XScopeGuard const guard {
        [d]()noexcept{ d->m_inExec_ = true; },
        [d]()noexcept{ d->m_inExec_ = {}; }
    };

    while (!d->m_exits_.loadAcquire())
    { d->m_eventDispatcher->processEvent(); }
}

void EventLoop::quit() noexcept
{ d_func()->m_exits_.storeRelease(true); }

bool EventLoop::postEvent(event_fn fn,EventLoop * const el) noexcept {
    auto const current{ el ? el->m_d_ptr_->m_eventDispatcher : EventDispatcherTls::getCurrent() };
    if (!current) { return {}; }
    current->postEvent(std::move(fn));
    return true;
}

bool EventLoop::postTimeEvent(uint32_t const ms,event_fn fn ,EventLoop * const el) noexcept {
    auto const current{ el ? el->m_d_ptr_->m_eventDispatcher : EventDispatcherTls::getCurrent() };
    if (!current) { return {}; }
    current->postTimeEvent(std::move(fn),ms);
    return true;
}

bool EventLoop::postTimeEvent(std::chrono::milliseconds const ms,event_fn fn,EventLoop * const el) noexcept {
    auto const current{ el ? el->m_d_ptr_->m_eventDispatcher : EventDispatcherTls::getCurrent() };
    if (!current) { return {}; }
    current->postTimeEvent(std::move(fn),ms);
    return true;
}

#endif
