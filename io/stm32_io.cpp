#include "stm32_io.hpp"
#include "stm32f4xx_ll_gpio.h"

#define CHANGE_(ptr) static_cast<GPIO_TypeDef*>(ptr)


void Stm32_io::_move_(Stm32_io &obj) noexcept {
    m_port_ = std::exchange(obj.m_port_, {});
    m_pin_ = std::exchange(obj.m_pin_, {});
}

Stm32_io::Stm32_io(void *port, const uint32_t &pin) : m_port_(port), m_pin_(pin) {
}

Stm32_io::Stm32_io(Stm32_io &&obj) noexcept {
    _move_(obj);
}

Stm32_io & Stm32_io::operator=(Stm32_io &&obj) noexcept {
    if (this != &obj) {
        _move_(obj);
    }
    return *this;
}

void Stm32_io::Set() const {
    CHECK_EMPTY(m_port_, return);
    LL_GPIO_SetOutputPin(CHANGE_(m_port_), m_pin_);
}

void Stm32_io::Reset() const {
    CHECK_EMPTY(m_port_, return);
    LL_GPIO_ResetOutputPin(CHANGE_(m_port_), m_pin_);
}

void Stm32_io::Dir(const bool &b) const {
    CHECK_EMPTY(m_port_, return);
    if (b) {
        LL_GPIO_SetPinMode(CHANGE_(m_port_), m_pin_, LL_GPIO_MODE_OUTPUT);
    } else {
        LL_GPIO_SetPinMode(CHANGE_(m_port_), m_pin_, LL_GPIO_MODE_INPUT);
    }
}

void Stm32_io::Toggle() const {
    CHECK_EMPTY(m_port_, return);
    LL_GPIO_TogglePin(CHANGE_(m_port_), m_pin_);
}

bool Stm32_io::Read() const {
    return m_port_ && LL_GPIO_IsInputPinSet(CHANGE_(m_port_), m_pin_);
}
