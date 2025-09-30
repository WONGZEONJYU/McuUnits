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

    void setNextResponse(XCORAbstract * const next) const noexcept
    { m_next_.storeRelease(next); }

    virtual void request(Arguments const & args) const {
        if (auto const next { dynamic_cast<XCOR<Const,Args...> * >(m_next_.loadAcquire()) })
        { next->responseHandler(args); return ; }
        if (auto const next { dynamic_cast<XCOR<NonConst,Args...> * >(m_next_.loadAcquire()) })
        { next->responseHandler(args); }
    }

    virtual ~XCORAbstract()
    { m_next_.storeRelease({}); }

    XCORAbstract(XCORAbstract && o) noexcept
    { swap(o); }

    XCORAbstract& operator=(XCORAbstract && o) noexcept
    { XCORAbstract {std::move(o)}.swap(*this); return *this; }

private:
    XCORAbstract() = default;

    void swap(XCORAbstract const & o) const noexcept {
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
    XCOR(XCOR && ) = default;
    XCOR & operator=(XCOR && ) = default;
    ~XCOR() override = default;

protected:
    virtual void responseHandler(Arguments const &) const {}
};

template<typename ... Args>
class XCOR<NonConst,Args...> : public XCORAbstract<Args...> {
    template<typename ...> friend class XCORAbstract;

public:
    using Arguments = XCORAbstract<Args...>::Arguments;
    constexpr XCOR() = default;
    XCOR(XCOR && ) = default;
    XCOR & operator=(XCOR && ) = default;
    ~XCOR() override = default;

protected:
    virtual void responseHandler(Arguments const &) {}
};

#endif
