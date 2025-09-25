#ifndef CHAR_DEV_BASE_HPP
#define CHAR_DEV_BASE_HPP 1

#include <memory>

class CharDevBase {
protected:
    CharDevBase() = default;
public:
    virtual ~CharDevBase() = default;

    virtual int64_t read(void *, std::size_t, int64_t) = 0;

    virtual int64_t read(void *, std::size_t, int64_t) const = 0;

    virtual int64_t write(const void *, std::size_t, int64_t) = 0;

    virtual int64_t write(const void *, std::size_t, int64_t) const = 0;

    virtual void startDMAReceive() {}

protected:
    virtual int64_t isrIn(void *, std::size_t) { return 0; }

    virtual int64_t isrIn(void *, std::size_t) const { return 0; }

    virtual int64_t isrDMAIn(std::size_t) { return 0; }

    [[nodiscard]] virtual int64_t isrDMAIn(std::size_t) const { return 0; }

    virtual void clearInDMABuffer() {}
};

#endif
