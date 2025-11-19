#ifndef ABSTRACT_IIC_HPP
#define ABSTRACT_IIC_HPP 1

#include <chardevbase.hpp>

class AbstractGPIO;

class AbstractIIC : public CharDevBase {

public:
    ~AbstractIIC() override = default;
    virtual void start() const noexcept;
    virtual void stop() const noexcept;
    virtual void sendAck(bool) const noexcept;
    [[nodiscard]] virtual bool ack() const noexcept;
    [[nodiscard]] virtual bool send(uint8_t d,bool ack) const noexcept;
    [[nodiscard]] virtual uint8_t recv(bool ack) const noexcept;

    int64_t read(void *, std::size_t, int64_t) noexcept override;
    int64_t read(void *, std::size_t, int64_t) const noexcept override;
    int64_t write(const void *, std::size_t, int64_t) noexcept override;
    int64_t write(const void *, std::size_t, int64_t) const noexcept override;

protected:
    explicit AbstractIIC() = default;

private:
    void sdaDir(bool) const noexcept;
    [[nodiscard]] virtual AbstractGPIO const & sclPort() const noexcept = 0;
    [[nodiscard]] virtual AbstractGPIO const & sdaPort() const noexcept = 0;
    virtual void delay() const noexcept;
};

#endif
