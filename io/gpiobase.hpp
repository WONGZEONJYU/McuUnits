#ifndef GPIO_BASE_HPP
#define GPIO_BASE_HPP 1

#include <xmemory.hpp>

class GPIOBase {

protected:
    void * m_port_{};
    long unsigned int m_pin_{};
public:
    virtual ~GPIOBase() = default;
    virtual void set() const = 0;
    virtual void reset() const = 0;
    virtual void dir(bool) const = 0;
    virtual void toggle() const = 0;
    [[nodiscard]] virtual bool read() const = 0;

protected:
    explicit GPIOBase(void * = {},uint32_t = {});
    void swap(GPIOBase &) noexcept;
    void copy(GPIOBase const &) noexcept;
};

#endif
