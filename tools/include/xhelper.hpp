#ifndef XHELPER_HPP
#define XHELPER_HPP 1

#include <wglobal.hpp>
#include <xatomic.hpp>
#include <functional>
#include <utility>

#define FUNC_SIGNATURE __PRETTY_FUNCTION__

#define GET_STR(args) #args

template<typename F>
class [[maybe_unused]] Destroyer final{
    W_DISABLE_COPY(Destroyer)
    F fn{};
    XAtomicBool is_destroy{};

public:
    constexpr explicit Destroyer(F && f):fn(std::move(f)){}

    constexpr void destroy() {
        if (!is_destroy.loadRelaxed()) {
            is_destroy.storeRelaxed(true);
            fn();
        }
    }

    constexpr ~Destroyer()
    { destroy(); }
};

template<typename F2>
class XRAII final {
    W_DISABLE_COPY(XRAII)
    F2 m_f2{};
    XAtomicBool m_is_destroy{};

public:
    template<typename F1>
    [[maybe_unused]] constexpr explicit XRAII(F1 && f1,F2 && f2): m_f2(std::move(f2))
    { f1(); }

    constexpr void destroy(){
        if (!m_is_destroy.loadRelaxed()){
            m_is_destroy.storeRelaxed(true);
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

enum class NonConst{};
enum class Const{};

#endif
