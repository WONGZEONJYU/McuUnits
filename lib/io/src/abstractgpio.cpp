#include <abstractgpio.hpp>

AbstractGPIO::AbstractGPIO(void * const port, uint32_t const pin)
:m_port_(port),m_pin_(pin) {}

void AbstractGPIO::swap(AbstractGPIO & o) noexcept {
    std::swap(m_port_, o.m_port_);
    std::swap(m_pin_, o.m_pin_);
}

void AbstractGPIO::copy(AbstractGPIO const & o) noexcept
{ m_port_ = o.m_port_; m_pin_ = o.m_pin_; }

AbstractGPIO::operator bool() const noexcept
{ return m_port_ && m_pin_; }

bool AbstractGPIO::operator!() const noexcept
{ return !operator bool(); }

void AbstractGPIO::setMode(GPIOMode) const {}

void AbstractGPIO::setPull(GPIOPull) const {}

GPIOMode AbstractGPIO::mode() const
{ return GPIOMode::UNKNOWN; }

GPIOPull AbstractGPIO::pull() const
{ return GPIOPull::UNKNOWN; }

void AbstractGPIO::setOutputType(GPIOOutPutType) const {}

GPIOOutPutType AbstractGPIO::outputType() const
{ return GPIOOutPutType::UNKNOWN; }

void AbstractGPIO::setSpeed(GPIOSpeed) const {}

GPIOSpeed AbstractGPIO::speed() const
{ return GPIOSpeed::UNKNOWN; }

void AbstractGPIO::setGPIO(void * const port, uint32_t const pin) noexcept
{ m_port_ = port; m_pin_ = pin; }
