#include <gpiobase.hpp>
#include <utility>

void GPIOBase::swap(GPIOBase & o) noexcept {
    std::swap(m_port_, o.m_port_);
    std::swap(m_gpio_, o.m_gpio_);
}

void GPIOBase::copy(GPIOBase const & o) noexcept
{ m_port_ = o.m_port_; m_gpio_ = o.m_gpio_; }
