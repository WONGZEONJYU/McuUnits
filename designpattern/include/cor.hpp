#ifndef X_COR_HPP
#define X_COR_HPP 1

#include <wglobal.hpp>
#include <xhelper.hpp>
#include <xatomic.hpp>
#include <tuple>

template<typename ,typename...> class XCOR;

template<typename ...Args>
class XCORAbstract {
    W_DISABLE_COPY(XCORAbstract)
    mutable XAtomicPointer<XCORAbstract> m_next_{};

public:
    using Arguments = std::tuple<Args...>;

    constexpr void setNextResponse(XCORAbstract * const next) const noexcept
    { m_next_.storeRelease(next); }

    constexpr virtual void request(Arguments && args) const {
        if (auto const next { dynamic_cast<XCOR<Const,Args...> * >(m_next_.loadAcquire()) })
        { next->responseHandler(std::forward<decltype(args)>(args)); return ; }
        if (auto const next { dynamic_cast<XCOR<NonConst,Args...> * >(m_next_.loadAcquire()) })
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

    template<typename,typename ...> friend class XCOR;
};

template<typename ... Args>
class XCOR<Const,Args...> : public XCORAbstract<Args...> {
    template<typename ...> friend class XCORAbstract;

public:
    using Arguments = XCORAbstract<Args...>::Arguments;
    constexpr XCOR() = default;
    constexpr XCOR(XCOR && ) = default;
    constexpr XCOR & operator=(XCOR && ) = default;
    constexpr ~XCOR() override = default;

protected:
    constexpr virtual void responseHandler(Arguments &&) const {}
};

template<typename ... Args>
class XCOR<NonConst,Args...> : public XCORAbstract<Args...> {
    template<typename ...> friend class XCORAbstract;

public:
    using Arguments = XCORAbstract<Args...>::Arguments;
    constexpr XCOR() = default;
    constexpr XCOR(XCOR && ) = default;
    constexpr XCOR & operator=(XCOR && ) = default;
    constexpr ~XCOR() override = default;

protected:
    constexpr virtual void responseHandler(Arguments &&) {}
};

#endif
