#ifndef X_GENERIC_ATOMIC_HPP
#define X_GENERIC_ATOMIC_HPP 1

#include <xtypes.hpp>

template<int Size> struct XAtomicOpsSupport {
    enum { IsSupported = Size == sizeof(int) || Size == sizeof(xptrdiff) };
};

template <typename T> struct XAtomicAdditiveType {
    using AdditiveT = T;
    [[maybe_unused]] static constexpr auto AddScale {1};
};

template <typename T> struct XAtomicAdditiveType<T *> {
    using AdditiveT = xptrdiff;
    [[maybe_unused]] static constexpr auto AddScale
    { sizeof(T) };
};

#endif
