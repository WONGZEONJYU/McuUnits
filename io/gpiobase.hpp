#ifndef GPIO_BASE_HPP
#define GPIO_BASE_HPP 1

#include <xmemory.hpp>

enum class GPIOMode : int {
    INPUT,OUTPUT,ALTERNATE,ANALOG,UNKNOWN = -1
};

enum class GPIOPull : int {
    NONE,PULL_UP,PULL_DOWN,UNKNOWN = -1
};

enum class GPIOOutPutType : int {
    PUSHPULL,OPEN_DRAIN,UNKNOWN = -1
};

enum class GPIOSpeed : int {
    SPEED_FREQ_LOW,SPEED_FREQ_MEDIUM,
    SPEED_FREQ_HIGH,SPEED_FREQ_VERY_HIGH,
    UNKNOWN = -1
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
    [[nodiscard]] virtual bool read(bool * = {}) const = 0;
    virtual void setMode(GPIOMode) const;
    virtual void setPull(GPIOPull) const;
    [[nodiscard]] virtual GPIOMode mode() const;
    [[nodiscard]] virtual GPIOPull pull() const;
    virtual void setOutputType(GPIOOutPutType) const;
    [[nodiscard]] virtual GPIOOutPutType outputType() const;
    virtual void setSpeed(GPIOSpeed) const;
    [[nodiscard]] virtual GPIOSpeed speed() const;

protected:
    explicit GPIOBase(void * = {},uint32_t = {});
    GPIOBase(GPIOBase const &) = default;
    GPIOBase(GPIOBase &&) = default;
    GPIOBase & operator=(GPIOBase const &) = default;
    GPIOBase & operator=(GPIOBase && ) = default;
    void swap(GPIOBase &) noexcept;
    void copy(GPIOBase const &) noexcept;
    operator bool() const noexcept;
    bool operator!() const noexcept;
};

#endif
