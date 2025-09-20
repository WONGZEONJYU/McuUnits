#include <gpiobase.hpp>

GPIOBase::GPIOBase(void * const port, uint32_t const pin)
:m_port_(port),m_pin_(pin) {}

void GPIOBase::swap(GPIOBase & o) noexcept {
    std::swap(m_port_, o.m_port_);
    std::swap(m_pin_, o.m_pin_);
}

void GPIOBase::copy(GPIOBase const & o) noexcept
{ m_port_ = o.m_port_; m_pin_ = o.m_pin_; }

GPIOBase::operator bool() const noexcept
{ return m_port_ && m_pin_; }

bool GPIOBase::operator!() const noexcept
{ return !operator bool(); }

void GPIOBase::setMode(GPIOMode) const {}

void GPIOBase::setPull(GPIOPull) const {}

GPIOMode GPIOBase::mode() const
{ return GPIOMode::UNKNOWN; }

GPIOPull GPIOBase::pull() const
{ return GPIOPull::UNKNOWN; }

void GPIOBase::setOutputType(GPIOOutPutType) const {}

GPIOOutPutType GPIOBase::outputType() const
{ return GPIOOutPutType::UNKNOWN; }

void GPIOBase::setSpeed(GPIOSpeed) const {}

GPIOSpeed GPIOBase::speed() const
{ return GPIOSpeed::UNKNOWN; }

void GPIOBase::setGPIO(void * const port, uint32_t const pin) noexcept
{ m_port_ = port; m_pin_ = pin; }
