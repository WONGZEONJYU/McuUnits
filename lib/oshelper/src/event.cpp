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

    class EventImpl final {

        using timeEvent_t = std::pair<ElapsedTimer, event_fn>;

        Mutex mutable m_events_mtx_{},m_timeEvents_mtx_{};
        XDeque<event_fn> m_events_{};
        XDeque<timeEvent_t> m_delayEvents_{};
        XDeque<XAtomicInt> m_exits_{};
        XAtomicInt m_ref_{1};
        void * m_ownershipThread_ { xTaskGetCurrentTaskHandle() };
        bool m_inExec_{};

        struct RefGuard final {
            explicit RefGuard(EventImpl * const obj) noexcept :m_obj_{obj}
            { obj->ref();}
            ~RefGuard() noexcept { m_obj_->deref(); }
        private:
            EventImpl * m_obj_{};
        };

    public:
        explicit EventImpl() = default;
        ~EventImpl() = default;

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

        void exec() noexcept {
            configASSERT(xTaskGetCurrentTaskHandle() == m_ownershipThread_);

            if (m_inExec_) { return; }

            XScopeGuard const guard{[this]()noexcept{ m_exits_.emplace_front(); m_inExec_ = true;}
                                ,[this]()noexcept{ m_exits_.pop_front(); m_inExec_ = {}; }
            };

            auto && exit{ std::as_const(m_exits_.front()) };
            while (!exit.loadAcquire()) {
                processTimeEvent();
                processEvent();
                XAbstractThread::yield();
            }
        }

        void quit() noexcept
        { m_exits_.front().storeRelease(true); }

    private:
        void processEvent() noexcept {
            auto const get {
                [this]() noexcept -> event_fn {
                    std::unique_lock lk{m_events_mtx_};
                    if (m_events_.empty()) { return {}; }
                    auto ret{ std::move(m_events_.front()) };
                    m_events_.pop_front();
                    return ret;
                }
            };

            while (!m_exits_.front().loadAcquire()) {
                auto const event{ get() };
                if (!event) { break; }
                event();
            }
        }

        void processTimeEvent() noexcept {
            decltype(m_delayEvents_) delayEvents{};
            std::unique_lock lk {m_timeEvents_mtx_};
            while (!m_exits_.front().loadAcquire() && !m_delayEvents_.empty()) {
                auto evt{ std::move(m_delayEvents_.front()) };
                m_delayEvents_.pop_front();
                auto && [t,e] { evt };
                t ? m_events_.push_back(std::move(e)) : delayEvents.push_back(std::move(evt));
            }
            if (delayEvents.empty()) { return; }
            m_delayEvents_.insert(m_delayEvents_.cend(),delayEvents.cbegin(),delayEvents.cend());
        }
    };

    class EventImplTls final {

        static void setCurrent(void * const v) noexcept
        { vTaskSetThreadLocalStoragePointer({},{},v); }

    public:
        static auto getCurrent() noexcept
        { return static_cast<EventImpl*>(pvTaskGetThreadLocalStoragePointer({},{})); }

        static EventImpl * current() noexcept {
            TaskCriticalArea t{};
            auto h{ getCurrent() };
            if (!h) {
                h = makeUnique<EventImpl>().release();
                if (h) { setCurrent(h); }
            }
            return h;
        }

        static void remove(EventImpl * const ev) noexcept {
            TaskCriticalArea t{};
            if (!ev ||
                ev->ownershipThread() != xTaskGetCurrentTaskHandle() ||
                ev != getCurrent() ||
                ev->deref()
            ) { return; }
            setCurrent({});
            XUniquePtr<EventImpl> const del { ev };
        }
    };

}

class EventLoopPrivate final {

public:
    EventLoop * q_ptr{};
    EventImpl * current{};
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
    auto const evt{ EventImplTls::current() };
    configASSERT(evt);
    d->current = evt;
}

EventLoop::~EventLoop()
{ EventImplTls::remove(d_func()->current); }

void EventLoop::post(event_fn fn) noexcept
{ d_func()->current->postEvent(std::move(fn)); }

void EventLoop::postTimeEvt(uint32_t const ms,event_fn fn) noexcept
{ d_func()->current->postTimeEvent(std::move(fn),ms); }

void EventLoop::postTimeEvt(std::chrono::milliseconds const ms,event_fn fn) noexcept
{ d_func()->current->postTimeEvent(std::move(fn),ms); }

void EventLoop::exec() noexcept
{ d_func()->current->exec(); }

void EventLoop::quit() noexcept
{ d_func()->current->quit(); }

bool EventLoop::postEvent(event_fn fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return {}; }
    current->postEvent(std::move(fn));
    return true;
}

bool EventLoop::postTimeEvent(uint32_t const ms,event_fn fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return {}; }
    current->postTimeEvent(std::move(fn),ms);
    return true;
}

bool EventLoop::postTimeEvent(std::chrono::milliseconds const ms,event_fn fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return {}; }
    current->postTimeEvent(std::move(fn),ms);
    return true;
}

#endif
