#ifndef ABSTRACT_IIC_HPP
#define ABSTRACT_IIC_HPP 1

#include <memory>
#include <chardevbase.hpp>

class GPIOBase;

class AbstractIIC  : public CharDevBase{

protected:
    GPIOBase * m_scl_{},* m_sda_{};

public:
    ~AbstractIIC() override;
    virtual void start() const noexcept;
    virtual void stop() const noexcept;
    virtual void sendAck(bool) const noexcept;
    [[nodiscard]] virtual bool ack() const noexcept;
    [[nodiscard]] virtual bool send(uint8_t d,bool ack) const noexcept;
    [[nodiscard]] virtual uint8_t recv(bool ack) const noexcept;

    int64_t read(void *, std::size_t, int64_t) noexcept override ;
    int64_t read(void *, std::size_t, int64_t) const noexcept override;
    int64_t write(const void *, std::size_t, int64_t) noexcept override;
    int64_t write(const void *, std::size_t, int64_t) const noexcept override;

protected:
    static void delay(std::size_t = 50) noexcept;
    explicit AbstractIIC(GPIOBase &,GPIOBase &);
    explicit AbstractIIC() = default;
    void sdaDir(bool) const noexcept;
    [[nodiscard]] constexpr bool check() const noexcept
    { return m_scl_ && m_sda_; }
};

#endif
