#ifndef X_COR_HPP
#define X_COR_HPP 1

#include <wglobal.hpp>
#include <xhelper.hpp>
#include <xatomic.hpp>

template<typename ...Args>
using XCORArgs = std::tuple<Args...>;

template<typename,typename> class XCOR;

template<typename Args>
class XCORAbstract {
    W_DISABLE_COPY(XCORAbstract)
    mutable XAtomicPointer<XCORAbstract> m_next_{};
    static_assert(is_tuple_v<Args>,"Args must be XCORArgs<...>!");

public:
    using Arguments = Args;

    constexpr void setNextResponse(XCORAbstract * const next) const noexcept
    { m_next_.storeRelease(next); }

    [[nodiscard]] constexpr bool NextResponseExist() const noexcept
    { return m_next_.loadAcquire(); }

    [[nodiscard]] constexpr operator bool() const noexcept
    { return m_next_.loadAcquire(); }

    virtual void request(Arguments && args) const {
        if (auto const next { dynamic_cast<XCOR<Const,Arguments> * >(m_next_.loadAcquire()) })
        { next->responseHandler(std::forward<decltype(args)>(args)); return ; }
        if (auto const next { dynamic_cast<XCOR<NonConst,Arguments> * >(m_next_.loadAcquire()) })
        { next->responseHandler(std::forward<decltype(args)>(args)); }
    }

    constexpr virtual ~XCORAbstract()
    { m_next_.storeRelease({}); }

    constexpr XCORAbstract(XCORAbstract && o) noexcept
    { swap(o); }

    constexpr XCORAbstract& operator=(XCORAbstract && o) noexcept
    { XCORAbstract {std::move(o)}.swap(*this); return *this; }

private:
    constexpr XCORAbstract() = default;

    constexpr void swap(XCORAbstract const & o) const noexcept {
        auto const self { m_next_.loadAcquire() };
        m_next_.storeRelease(o.m_next_.loadAcquire());
        o.m_next_.storeRelease(self);
    }

    template<typename,typename> friend class XCOR;
};

template<typename Args>
class XCOR<Const,Args> : public XCORAbstract<Args> {
    using Base = XCORAbstract<Args>;
    template<typename> friend class XCORAbstract;

public:
    using Arguments = Base::Arguments;
    constexpr XCOR() = default;
    constexpr XCOR(XCOR && ) = default;
    constexpr XCOR & operator=(XCOR && ) = default;
    constexpr ~XCOR() override = default;

protected:
    constexpr virtual void responseHandler(Arguments &&) const {}
};

template<typename Args>
class XCOR<NonConst,Args> : public XCORAbstract<Args> {
    using Base = XCORAbstract<Args>;
    template<typename> friend class XCORAbstract;

public:
    using Arguments = Base::Arguments;
    constexpr XCOR() = default;
    constexpr XCOR(XCOR && ) = default;
    constexpr XCOR & operator=(XCOR && ) = default;
    constexpr ~XCOR() override = default;

protected:
    constexpr virtual void responseHandler(Arguments &&) {}
};

#endif
