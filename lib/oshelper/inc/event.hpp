#ifndef EVENT_HPP
#define EVENT_HPP

#include <functional>
#include <xmemory.hpp>

class EventLoopPrivate;

class EventLoop final {

    XUniquePtr<EventLoopPrivate> m_d_ptr_;
    W_DECLARE_PRIVATE_D(m_d_ptr_,EventLoop)
    W_DISABLE_COPY_MOVE(EventLoop)

public:
    explicit EventLoop();
    ~EventLoop();

    void post(std::function<void()> fn) noexcept;
    void postTimeEvt(uint32_t,std::function<void()>) noexcept;
    void postTimeEvt(std::chrono::milliseconds,std::function<void()>) noexcept;

    static void postEvent(std::function<void()>) noexcept;
    static void postTimeEvent(uint32_t,std::function<void()>) noexcept;
    static void postTimeEvent(std::chrono::milliseconds,std::function<void()>) noexcept;

    void exec() noexcept;
    void quit() noexcept;
};

#endif
