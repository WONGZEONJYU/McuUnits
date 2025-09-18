#ifndef FC_BURNER_STM32_IO_HPP
#define FC_BURNER_STM32_IO_HPP

#include "xhelper.hpp"

class Stm32_io {
    X_DISABLE_COPY(Stm32_io)
    void _move_(Stm32_io &) noexcept;

public:
    Stm32_io() = default;
    Stm32_io(void * port, const uint32_t &pin);
    Stm32_io(Stm32_io &&obj) noexcept;
    Stm32_io& operator=(Stm32_io && ) noexcept;

    void Set() const;
    void Reset() const;
    void Dir(const bool &b) const;
    void Toggle() const;
    [[nodiscard]] bool Read() const;

private:
    void* m_port_{};
    uint32_t m_pin_{};
};

#endif //FC_BURNER_STM32_IO_HPP