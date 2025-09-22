#ifndef X_TASK_BASE_HPP
#define X_TASK_BASE_HPP 1

#include <xthread.hpp>

class XTaskBase {
    X_DISABLE_COPY_MOVE(XTaskBase)
    XThreadDynamic m_th_;
    std::atomic<XTaskBase*> m_next_{};
protected:
    std::atomic_bool m_is_exit_{};
public:
    virtual void Strat();
    virtual void Stop();
    virtual void Exit();

    void Set_Priority(const uint32_t &p) const noexcept;

    void set_next( XTaskBase * const next) noexcept
    { m_next_ = next; }

    virtual void Next(void * const arg) noexcept {
        if (m_next_.load(std::memory_order_acquire)) {
            m_next_.load(std::memory_order_relaxed)->Do(arg);
        }
    }

    virtual void Do(void * const arg) {}

    virtual ~XTaskBase();
protected:
    explicit XTaskBase() = default;

private:
    virtual void Main() = 0;
    void _stop_();
    void _exit_() { m_is_exit_.store(true,std::memory_order_release); }
};

#endif
