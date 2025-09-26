#ifndef X_TASK_BASE_HPP
#define X_TASK_BASE_HPP 1

#include <xthread.hpp>

class XTaskBase {
    X_DISABLE_COPY_MOVE(XTaskBase)
    XThreadDynamic m_th_;
    std::atomic<XTaskBase*> m_next_{};
    std::atomic_bool m_isRunning_{};

public:
    virtual void start(std::size_t stack_depth = 2048) noexcept;
    virtual void stop() noexcept;
    virtual void exit() noexcept;
    void setPriority(uint32_t) const noexcept;
    void setNext(XTaskBase *) noexcept;
    virtual void next(void * ) noexcept;
    virtual void handleRequest(void *) {}
    virtual ~XTaskBase();
    [[nodiscard]] bool isRunning() const noexcept;

protected:
    explicit XTaskBase() = default;

private:
    virtual void run() = 0;
    void stop_() noexcept;
    void exit_() noexcept;
};

#endif
