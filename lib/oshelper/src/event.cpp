#include <event.hpp>
#include <cassert>
#include <mutex.hpp>
#include <xcontainer.hpp>
#include <timeout.hpp>
#include <xatomic.hpp>
#include <xabstractthread.hpp>
#include <xscopeguard.hpp>

namespace {

    class EventImpl final {

        using event_fn = std::function<void()>;
        using timeEvent_t = std::pair<ElapsedTimer, event_fn>;

        Mutex mutable m_events_mtx{},m_timeEvents_mtx{};
        XDeque<event_fn> m_events{};
        XDeque<timeEvent_t> m_timeEvents{};
        XList<XAtomicInt> m_exits{};
        XAtomicInt m_ref{1};

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

        void ref() noexcept
        { m_ref.ref(); }

        bool deref() noexcept
        { return m_ref.deref(); }

        void postEvent(std::function<void()> fn) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_events_mtx};
            m_events.push_back(std::move(fn));
        }

        void postTimeEvent(std::function<void()> fn,uint32_t ms) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_timeEvents_mtx};
            m_timeEvents.emplace_back(ms,std::move(fn));
        }

        void postTimeEvent(std::function<void()> fn,std::chrono::milliseconds ms) noexcept {
            RefGuard g {this};
            std::unique_lock lk {m_timeEvents_mtx};
            m_timeEvents.emplace_back(ms,std::move(fn));
        }

        void exec() noexcept {

            XScopeGuard const guard{[this]()noexcept{ m_exits.emplace_front(); }
                                ,[this]()noexcept{ m_exits.pop_front();}
            };

            auto && exit{ std::as_const(m_exits.front()) };
            while (!exit.loadAcquire()) {
                processEvent();
                processTimeEvent();
                XAbstractThread::yield();
            }
        }

        void quit() noexcept
        { m_exits.front().storeRelease(true); }

    private:
        void processEvent() noexcept {

            auto const get {
                [this]() noexcept -> event_fn {
                    std::unique_lock lk{m_events_mtx};
                    if (m_events.empty()) { return {}; }
                    auto ret{ std::move(m_events.front()) };
                    m_events.pop_front();
                    return ret;
                }
            };

            while (!m_exits.front().loadAcquire()) {
                auto const event{ get() };
                if (!event) { break; }
                event();
            }
        }

        void processTimeEvent() noexcept {

            auto const timeOutEvent {
                [this]() noexcept -> event_fn {
                    std::unique_lock lk {m_timeEvents_mtx};
                    if (m_timeEvents.empty()) { return {}; }
                    auto evt{ std::move(m_timeEvents.front()) };
                    m_timeEvents.pop_front();
                    if (!evt.first) {
                        m_timeEvents.push_back(std::move(evt));
                        return {};
                    }
                    return std::move(evt.second);
                }()
            };

            if (timeOutEvent) { timeOutEvent(); }
        }
    };

    class EventImplTls final {
        XMap<TaskHandle_t,XUniquePtr<EventImpl>> g_events {};
        Mutex g_mutex {};

    public:
        ~EventImplTls() = default;

        static EventImpl * current() noexcept {
            auto h{ getCurrent() };
            if (!h) { if (h = instance().get();h) { setCurrent(h); } }
            return h;
        }

        static EventImpl * getCurrent() noexcept
        { return static_cast<EventImpl*>(pvTaskGetThreadLocalStoragePointer({},{})); }

        static void remove(EventImpl * const evt) noexcept
        { instance().erase(evt); }

    private:
        static EventImplTls & instance() noexcept {
            static EventImplTls eventImplTls {};
            return eventImplTls;
        }

        EventImpl * get() noexcept {
            auto const th { xTaskGetCurrentTaskHandle() };
            std::unique_lock lk {g_mutex};
            auto && evt { g_events[th] };
            if (!evt) { makeUnique<EventImpl>().swap(evt); }
            if (!evt) { return {}; }
            evt->ref();
            return evt.get();
        }

        void erase(EventImpl * const evt) noexcept {
            auto const th{ xTaskGetCurrentTaskHandle() };
            if (!evt || evt->deref()) { return; }
            if (getCurrent() == evt) { setCurrent({}); }
            std::unique_lock lk {g_mutex};
            auto const it { g_events.find(th) };
            if (it == g_events.end() || it->second.get() != evt) { return; }
            g_events.erase(it);
        }

        static void setCurrent(void * const v) noexcept
        { vTaskSetThreadLocalStoragePointer({},{},v); }

        EventImplTls() = default;
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
    assert(m_d_ptr_);
    m_d_ptr_->q_ptr = this;
    W_D(EventLoop);
    auto const evt{ EventImplTls::current() };
    assert(evt);
    d->current = evt;
}

EventLoop::~EventLoop()
{ EventImplTls::remove(d_func()->current); }

void EventLoop::post(std::function<void()> fn) noexcept
{ d_func()->current->postEvent(std::move(fn)); }

void EventLoop::postTimeEvt(uint32_t const ms,std::function<void()> fn) noexcept
{ d_func()->current->postTimeEvent(std::move(fn),ms); }

void EventLoop::postTimeEvt(std::chrono::milliseconds const ms,std::function<void()> fn) noexcept
{ d_func()->current->postTimeEvent(std::move(fn),ms); }

void EventLoop::postEvent(std::function<void()> fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return; }
    current->postEvent(std::move(fn));
}

void EventLoop::postTimeEvent(uint32_t const ms,std::function<void()> fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return; }
    current->postTimeEvent(std::move(fn),ms);
}

void EventLoop::postTimeEvent(std::chrono::milliseconds const ms,std::function<void()> fn) noexcept {
    auto const current{ EventImplTls::getCurrent() };
    if (!current) { return; }
    current->postTimeEvent(std::move(fn),ms);
}

void EventLoop::exec() noexcept
{ d_func()->current->exec(); }

void EventLoop::quit() noexcept
{ d_func()->current->quit(); }
