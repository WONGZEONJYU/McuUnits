#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional>
#include <xmemory.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

using event_fn = std::function<void()>;

class EventLoopPrivate;

class EventLoop final {

    XUniquePtr<EventLoopPrivate> m_d_ptr_;
    W_DECLARE_PRIVATE_D(m_d_ptr_,EventLoop)
    W_DISABLE_COPY_MOVE(EventLoop)

public:
    explicit EventLoop();
    ~EventLoop();

    void post(event_fn fn) noexcept;
    void postTimeEvt(uint32_t,event_fn) noexcept;
    void postTimeEvt(std::chrono::milliseconds,event_fn) noexcept;

    void exec() noexcept;
    void quit() noexcept;

    static bool postEvent(event_fn) noexcept;
    static bool postTimeEvent(uint32_t,event_fn) noexcept;
    static bool postTimeEvent(std::chrono::milliseconds,event_fn) noexcept;
};

#endif

#endif
