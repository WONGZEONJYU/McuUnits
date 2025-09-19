#include <gpiobase.hpp>

void GPIOBase::setMode(GPIOMode) const {}

void GPIOBase::setPull(GPIOPull) const {}

GPIOMode GPIOBase::mode() const
{ return GPIOMode::UNKNOWN; }

GPIOPull GPIOBase::pull() const
{ return GPIOPull::UNKNOWN; }

GPIOBase::GPIOBase(void * const port, uint32_t const pin)
:m_port_(port),m_pin_(pin){}

void GPIOBase::swap(GPIOBase & o) noexcept {
    std::swap(m_port_, o.m_port_);
    std::swap(m_pin_, o.m_pin_);
}

void GPIOBase::copy(GPIOBase const & o) noexcept
{ m_port_ = o.m_port_; m_pin_ = o.m_pin_; }
