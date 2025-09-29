#ifndef XHELPER_HPP
#define XHELPER_HPP 1

#include <memory>
#include <wglobal.hpp>
#include <xatomic.hpp>

#define FUNC_SIGNATURE __PRETTY_FUNCTION__

#define GET_STR(args) #args

template<typename F>
class Destroyer {
    W_DISABLE_COPY(Destroyer)
    F m_fn_{};
    XAtomicBool is_destroy{};

public:
    constexpr explicit Destroyer(F && f)
    :m_fn_ { std::forward<F>(f) } {}

    void destroy() noexcept {
        if (!is_destroy.loadRelaxed()) {
            is_destroy.storeRelaxed(true);
            m_fn_();
        }
    }

    virtual ~Destroyer()
    { destroy(); }
};

template<typename F2>
class XRAII final : public Destroyer<F2> {
    W_DISABLE_COPY(XRAII)
    using Base = Destroyer<F2>;

public:
    template<typename F1>
    constexpr explicit XRAII(F1 && f1,F2 && f2) : Base(std::forward<F2>(f2))
    { f1(); }
    ~XRAII() override = default;
};

#define CCMRAM __attribute__((section(".ccmram")))
#define RAM  __attribute__((section(".ram")))
#define PACK __attribute__((packed));
#define INLINE __attribute__((__always_inline__))

#define CHECK_EMPTY(x,...) do { if(!x){__VA_ARGS__;} }while(false)

enum class NonConst{};
enum class Const{};

#endif
