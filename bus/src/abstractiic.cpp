#include <abstractiic.hpp>
#include <gpiobase.hpp>
#include <xhelper.hpp>

void AbstractIIC::delay(std::size_t cnt) noexcept
{ while (--cnt){} }

void AbstractIIC::start() const noexcept {
    CHECK_EMPTY(check(),return);
    sdaDir(true);
    m_scl_->set();
    delay();
    m_sda_->set();
    delay();
    m_sda_->reset();
    delay();
    m_scl_->reset();
    delay();
}

void AbstractIIC::stop() const noexcept {
    CHECK_EMPTY(check(),return);
    sdaDir(true);
    m_sda_->reset();
    delay();
    m_scl_->reset();
    delay();
    m_scl_->set();
    delay();
    m_sda_->set();
    delay();
}

void AbstractIIC::sendAck(bool const b) const noexcept {
    CHECK_EMPTY(check(),return);
    m_scl_->reset();
    delay();
    sdaDir(true);
    b ? m_sda_->reset() : m_sda_->set();
    delay();
    m_scl_->set();
    delay();
    m_scl_->reset();
    delay();
}

bool AbstractIIC::ack() const noexcept {
    CHECK_EMPTY(check(),return {});
    sdaDir({});
    m_scl_->reset();
    delay();
    m_sda_->set();
    delay();
    m_scl_->set();
    delay();
    auto time { 1000 };
    while (m_sda_->read().value_or(true))
    { if (!--time) { stop(); return {}; } }
    m_scl_->reset();
    delay();
    return true;
}

bool AbstractIIC::send(uint8_t d,bool const ack) const noexcept {
    CHECK_EMPTY(check(),return {});
    sdaDir(true);
    for (int i{};i < 8;++i) {
        m_scl_->reset();
        delay();
        d & 0x80 ? m_sda_->set() : m_sda_->reset();
        d <<= 1;
        delay();
        m_scl_->set();
        delay();
    }
    return !ack || this->ack();
}

uint8_t AbstractIIC::recv(bool const ack) const noexcept {
    CHECK_EMPTY(check(),return {});
    sdaDir({});
    m_scl_->reset();
    delay();
    m_sda_->set();
    uint8_t d {};
    for (int i{};i < 8;++i) {
        m_scl_->set();
        delay();
        d <<= 1;
        if (m_sda_->read().value_or(false)) { ++d; }
        m_scl_->reset();
        delay();
    }
    sendAck(ack);
    return d;
}

AbstractIIC::~AbstractIIC() = default;

AbstractIIC::AbstractIIC(GPIOBase & scl,GPIOBase & sda)
    : m_scl_(std::addressof(scl))
,m_sda_(std::addressof(sda))
{}

void AbstractIIC::sdaDir(bool const b) const noexcept{
    if (b) {
        m_sda_->setMode(GPIOMode::OUTPUT);
        m_sda_->setOutputType(GPIOOutPutType::PUSHPULL);
        m_sda_->setPull(GPIOPull::PULL_UP);
    }else {
        m_sda_->setMode(GPIOMode::INPUT);
        m_sda_->setPull(GPIOPull::NONE);
    }
}

int64_t AbstractIIC::read(void * const dst, std::size_t const len, int64_t) noexcept {
    CHECK_EMPTY(dst,return -1);

    XRAII const r {[this]{start();},[this]{stop();}};

    auto const pd { static_cast<uint8_t*>(dst) };

    uint8_t addr[] { static_cast<uint8_t>(pd[0] & 0xfe),pd[1] };

    for (auto const & item: addr)
    { if (!send(item,true)) { return -2; } }

    addr[0] |= 0x01; //read

    start();

    if (!send(addr[0],true)) { return -2; }

    int64_t c {};
    for (;c < len;++c) { pd[c] = recv(true); }
    return c;
}

int64_t AbstractIIC::read(void * const dst , std::size_t const len , int64_t) const noexcept
{ return const_cast<AbstractIIC*>(this)->read(dst,len,0); }

int64_t AbstractIIC::write(const void * const src , std::size_t const len, int64_t) noexcept {
    CHECK_EMPTY(src,return -1);
    XRAII const r {[this]{start();},[this]{stop();}};
    auto const pd { static_cast<const uint8_t*>(src) };
    int64_t c {};
    for (;c < len;++c) { if(send(pd[c] ,true)) { return -2; } }
    return c;
}

int64_t AbstractIIC::write(const void * const src , std::size_t const len , int64_t) const noexcept
{ return const_cast<AbstractIIC*>(this)->write(src,len,0); }
