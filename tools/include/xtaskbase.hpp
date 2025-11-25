#ifndef X_TASK_BASE_HPP
#define X_TASK_BASE_HPP 1

#include <xthread.hpp>

class XTaskBase {
    W_DISABLE_COPY_MOVE(XTaskBase)
    XThreadDynamic m_th_;
    XAtomicBool m_isRunning_{};

public:
    void start(std::size_t stack_depth = 1024) noexcept;
    void stop() noexcept;
    void exit() noexcept;
    void setPriority(uint32_t) const noexcept;
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
