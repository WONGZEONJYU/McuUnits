#include <abstractiic.hpp>
#include <abstractgpio.hpp>
#include <xhelper.hpp>
#include <xraii.hpp>

void AbstractIIC::start() const noexcept {
    sdaDir(true);
    sclPort().set();
    delay();
    sdaPort().set();
    delay();
    sdaPort().reset();
    delay();
    sclPort().reset();
    delay();
}

void AbstractIIC::stop() const noexcept {
    sdaDir(true);
    sdaPort().reset();
    delay();
    sclPort().reset();
    delay();
    sclPort().set();
    delay();
    sdaPort().set();
    delay();
}

void AbstractIIC::sendAck(bool const b) const noexcept {
    sclPort().reset();
    delay();
    sdaDir(true);
    b ? sdaPort().reset() : sdaPort().set();
    delay();
    sclPort().set();
    delay();
    sclPort().reset();
    delay();
}

bool AbstractIIC::ack() const noexcept {
    sdaDir({});
    sclPort().reset();
    delay();
    sdaPort().set();
    delay();
    sclPort().set();
    delay();
    auto time { 1000 };
    while (sdaPort().read().value_or(true))
    { if (!--time) { stop(); return {}; } }
    sclPort().reset();
    delay();
    return true;
}

bool AbstractIIC::send(uint8_t d,bool const ack) const noexcept {
    sdaDir(true);
    sclPort().reset();
    for (int i{};i < 8;++i) {
        d & 0x80 ? sdaPort().set() : sdaPort().reset();
        d <<= 1;
        delay();
        sclPort().set();
        delay();
        sclPort().reset();
    }
    return !ack || this->ack();
}

uint8_t AbstractIIC::recv(bool const ack) const noexcept {
    sdaDir({});
    sclPort().reset();
    delay();
    sclPort().set();
    uint8_t d {};
    for (int i{};i < 8;++i) {
        sclPort().set();
        delay();
        d <<= 1;
        if (sdaPort().read().value_or(false)) { ++d; }
        sclPort().reset();
        delay();
    }
    sendAck(ack);
    return d;
}

void AbstractIIC::sdaDir(bool const b) const noexcept{
    if (b) {
        sdaPort().setMode(GPIOMode::OUTPUT);
#if 0
        sdaPort().setOutputType(GPIOOutPutType::PUSHPULL);
        sdaPort().setPull(GPIOPull::PULL_UP);
#endif
    }else {
        sdaPort().setMode(GPIOMode::INPUT);
#if 0
        sdaPort().setPull(GPIOPull::NONE);
#endif
    }
}

int64_t AbstractIIC::read(void * const dst, std::size_t const len, int64_t) noexcept {

    CHECK_EMPTY(dst,return -1);

    X_RAII const r {[this]{start();},[this]{stop();}};

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
    X_RAII const r {[this]{start();},[this]{stop();}};
    auto const pd { static_cast<const uint8_t*>(src) };
    int64_t c {};
    for (;c < len;++c) { if(!send(pd[c] ,true)) { return -2; } }
    return c;
}

int64_t AbstractIIC::write(const void * const src , std::size_t const len , int64_t) const noexcept
{ return const_cast<AbstractIIC*>(this)->write(src,len,0); }

void AbstractIIC::delay() const noexcept
{ auto cnt { 5 }; while (--cnt){} }
