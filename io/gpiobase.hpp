#ifndef GPIO_BASE_HPP
#define GPIO_BASE_HPP 1

#include <xmemory.hpp>

enum class GPIOMode {
    INPUT,OUTPUT,ALTERNATE,ANALOG,UNKNOWN
};

enum class GPIOPull {
    NONE,PULL_UP,PULL_DOWN,UNKNOWN
};

class GPIOBase {

protected:
    void * m_port_{};
    uint32_t m_pin_{};
public:
    virtual ~GPIOBase() = default;
    virtual void set() const = 0;
    virtual void reset() const = 0;
    virtual void toggle() const = 0;
    [[nodiscard]] virtual bool read() const = 0;
    virtual void setMode(GPIOMode) const;
    virtual void setPull(GPIOPull) const;
    [[nodiscard]] virtual GPIOMode mode() const;
    [[nodiscard]] virtual GPIOPull pull() const;

protected:
    explicit GPIOBase(void * = {},uint32_t = {});
    void swap(GPIOBase &) noexcept;
    void copy(GPIOBase const &) noexcept;
};

#endif
