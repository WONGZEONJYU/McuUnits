#ifndef GPIO_BASE_HPP
#define GPIO_BASE_HPP 1

#include <optional>
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

class AbstractGPIO {

protected:
    void * m_port_{};
    uint32_t m_pin_{};

public:
    virtual ~AbstractGPIO() = default;
    virtual void set() const = 0;
    virtual void reset() const = 0;
    virtual void toggle() const = 0;
    [[nodiscard]] virtual std::optional<bool> read() const = 0;
    virtual void setMode(GPIOMode) const;
    virtual void setPull(GPIOPull) const;
    [[nodiscard]] virtual GPIOMode mode() const;
    [[nodiscard]] virtual GPIOPull pull() const;
    virtual void setOutputType(GPIOOutPutType) const;
    [[nodiscard]] virtual GPIOOutPutType outputType() const;
    virtual void setSpeed(GPIOSpeed) const;
    [[nodiscard]] virtual GPIOSpeed speed() const;
    void setGPIO(void *,uint32_t) noexcept;
    operator bool() const noexcept;
    bool operator!() const noexcept;

protected:
    explicit AbstractGPIO(void * = {},uint32_t = {});
    AbstractGPIO(AbstractGPIO const &) = default;
    AbstractGPIO(AbstractGPIO &&) = default;
    AbstractGPIO & operator=(AbstractGPIO const &) = default;
    AbstractGPIO & operator=(AbstractGPIO && ) = default;
    void swap(AbstractGPIO &) noexcept;
    void copy(AbstractGPIO const &) noexcept;
};

#endif
