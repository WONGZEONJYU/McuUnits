#ifndef XHELPER_HPP
#define XHELPER_HPP 1

#include <atomic>
#include <memory>
#include <functional>
#include <utility>

#define FUNC_SIGNATURE __PRETTY_FUNCTION__

namespace XHelper {
    [[maybe_unused]] static uint16_t swap16(const uint16_t &value) noexcept {
        return value >> 8 | value << 8;
    }
}

#define GET_STR(args) #args

#define X_DISABLE_COPY(Class) \
Class(const Class &) = delete;\
Class &operator=(const Class &) = delete;

#define X_DISABLE_COPY_MOVE(Class) \
X_DISABLE_COPY(Class) \
Class(Class &&) = delete; \
Class &operator=(Class &&) = delete;

template<typename F>
class [[maybe_unused]] Destroyer final{
    X_DISABLE_COPY(Destroyer)
    F fn{};
    std::atomic_bool is_destroy{};
public:
    constexpr explicit Destroyer(F &&f):fn(std::move(f)){}

    constexpr void destroy() {
        if (!is_destroy) {
            is_destroy = true;
            fn();
        }
    }

    constexpr ~Destroyer()
    { destroy(); }
};

template<typename F2>
class XRAII final {
    F2 m_f2{};
    std::atomic_bool m_is_destroy{};
    X_DISABLE_COPY(XRAII)
public:
    [[maybe_unused]] constexpr explicit XRAII(auto &&f1,F2 &&f2): m_f2(std::move(f2))
    { f1(); }

    constexpr void destroy(){
        if (!m_is_destroy){
            m_is_destroy = true;
            m_f2();
        }
    }

    constexpr ~XRAII()
    { destroy(); }
};

#define CCMRAM __attribute__((section(".ccmram")))
#define RAM  __attribute__((section(".ram")))
#define PACK __attribute__((packed));
#define INLINE __attribute__((__always_inline__))

#define CHECK_EMPTY(x,...) do { if(!x){__VA_ARGS__;} }while(false)

#endif
