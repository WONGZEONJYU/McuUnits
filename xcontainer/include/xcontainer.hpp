#ifndef X_CONTAINER_HPP
#define X_CONTAINER_HPP 1

#include <wglobal.hpp>
#include <xmemory.hpp>
#include <algorithm>
#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <memory>
#include <sstream>
#include <string>
#include <charconv>
#include <xatomic.hpp>

inline namespace XContainer {

    template<typename T,std::size_t Size_>
    class CircularBuffer final {

        W_DISABLE_COPY(CircularBuffer)

        static_assert(Size_ > 0,"Size_ is less than or equal zero!");

        mutable std::array<T,Size_> m_buf_{};

        mutable XAtomicInteger<std::size_t> m_w_{},m_r_{};

    public:
        using value_type = T;

        constexpr explicit CircularBuffer() = default;

        constexpr CircularBuffer(CircularBuffer && obj) noexcept
        { swap(obj); }

        constexpr CircularBuffer& operator=(CircularBuffer &&obj) noexcept
        { CircularBuffer(obj).swap(*this); return *this; }

        constexpr void swap(CircularBuffer const & obj) noexcept {
            m_buf_.swap(obj.m_buf_);
            auto const w { m_w_.loadRelaxed() }, r { m_r_.loadRelaxed() };
            m_w_.storeRelease(obj.m_w_.loadAcquire());
            m_r_.storeRelease(obj.m_r_.loadAcquire());
            obj.m_w_.storeRelease(w);
            obj.m_r_.storeRelease(r);
        }

        static constexpr auto capacity() noexcept
        { return Size_; }

        constexpr auto empty() const noexcept
        { return m_w_.loadRelaxed() == m_r_.loadRelaxed(); }

        constexpr auto full() const noexcept
        { return Size_ == m_w_.loadRelaxed() - m_r_.loadRelaxed(); }

        [[nodiscard]] constexpr int64_t readableSize() const noexcept {
            if (empty()) { return {}; }
            if (m_w_.loadRelaxed() > m_r_.loadRelaxed())
                { return m_w_.loadRelaxed() - m_r_.loadRelaxed(); }
            return Size_ - m_r_.loadRelaxed() + m_w_.loadRelaxed();
        }

        [[nodiscard]] constexpr int64_t writableSize() const noexcept {
            if (full()) { return int{}; }
            return Size_ - readableSize();
        }

        constexpr void clear() const noexcept {
            m_r_.storeRelease(0);
            m_w_.storeRelease(0);
        }

        constexpr bool write(value_type const & d) const noexcept {
            if (full()) { return {}; }
            m_buf_[m_w_.loadAcquire() % Size_] = d;
            m_w_.ref();
            return true;
        }

        constexpr bool write(value_type && d) const noexcept {
            if (full()) { return {}; }
            m_buf_[m_w_.loadAcquire() % Size_] = std::move(d);
            m_w_.ref();
            return true;
        }

        constexpr int64_t write(const value_type * const d,std::size_t const s) const noexcept {
            if (!d) { return -1; }
            if (full()) { return 0;}
            auto const canWrite{ std::min(static_cast<int64_t>(s),writableSize()) };
            int64_t c {};
            for (;c < canWrite;++c) { if (!write(d[c])) { break; } }
            return c;
        }

        constexpr bool read(value_type & d) const noexcept {
            if (empty()) { return {}; }
            d = m_buf_[m_r_.loadAcquire() % Size_];
            m_r_.ref();
            return true;
        }

        constexpr int64_t read(value_type * const d,std::size_t const s) const noexcept {
            if (!d) { return -1; }
            if (empty()) { return 0; }
            auto const canRead{ std::min(static_cast<int64_t>(s),readableSize()) };
            int64_t c {};
            for (;c < canRead;++c) { if (!read(d[c])){ break; } }
            return c;
        }

        constexpr int64_t peek(value_type * const d,std::size_t const s) const noexcept {
            if (!d) { return -1; }
            if (empty()) { return 0; }
            auto const canRead{ std::min(static_cast<int64_t>(s),readableSize()) };
            int64_t c {};
            auto pos{ m_r_.loadAcquire() % Size_ };
            for (;c < canRead;++c,++pos) { d[c] = m_buf_[pos]; }
            return c;
        }
    };

    template<typename Tp_>
    using XVector = std::vector< Tp_, XAllocator< Tp_ > >;

    using XVectorBool = std::vector<bool,XAllocator< bool >>;

    using XUByteArray = XVector<uint8_t>;

    using XByteArray = XVector<char>;

    template<typename Tp_>
    using XList = std::list< Tp_, XAllocator< Tp_ > >;

    template<typename Tp_>
    using XForwardList = std::forward_list<Tp_, XAllocator< Tp_ > >;

    template<typename Tp_>
    using XDeque = std::deque< Tp_, XAllocator< Tp_ > >;

    template<typename Key_>
    using XMultiset = std::multiset< Key_,std::less<Key_>,XAllocator<Key_> >;

    template<typename Key_,typename Tp_>
    using XMap = std::map< Key_,Tp_,std::less< Key_ >, XAllocator< std::pair<const Key_, Tp_> > >;

    template<typename Key_,typename Tp_>
    using XUnordered_map = std::unordered_map< Key_,Tp_,std::hash< Key_ >,std::equal_to< Key_ >, XAllocator< std::pair< const Key_, Tp_ > > >;

    template<typename Key_,typename Tp_>
    using XUnordered_multimap = std::unordered_multimap<Key_,Tp_,std::hash< Key_ >,std::equal_to< Key_ > , XAllocator< std::pair< const Key_, Tp_ > > >;

    template<typename Key_>
    using XUnordered_set = std::unordered_set< Key_,std::hash< Key_ >,std::equal_to< Key_ >,XAllocator< Key_ > >;

    template<typename Key_>
    using XUnordered_multiset = std::unordered_multiset< Key_ ,std::hash<Key_>,std::equal_to< Key_ >,XAllocator< Key_ > >;

    template<typename Key_>
    using XSet = std::set<Key_,std::less<Key_>,XAllocator<Key_>>;

    using XString = std::basic_string<char,std::char_traits<char>,XAllocator<char>>;

    using XWString = std::basic_string<wchar_t,std::char_traits<wchar_t>,XAllocator<wchar_t>>;

    using XU8String = std::basic_string<char8_t,std::char_traits<char8_t>,XAllocator<char8_t>>;

    using XU16String = std::basic_string<char16_t,std::char_traits<char16_t>,XAllocator<char16_t>>;

    using XU32String = std::basic_string<char32_t,std::char_traits<char32_t>,XAllocator<char32_t>>;

    using XStringStream  = std::basic_stringstream<char,std::char_traits<char>,XAllocator<char>>;

    using XIStringStream = std::basic_istringstream<char,std::char_traits<char>,XAllocator<char>>;

    using XOStringStream = std::basic_ostringstream<char,std::char_traits<char>,XAllocator<char>>;

    using XStringVector = XVector<XString>;

    using XStringList = XList<XString>;

    template<typename Con_>
    constexpr auto sliced(Con_ const & c ,std::size_t const s,std::size_t const e) noexcept -> Con_ {
        if (s >= c.size() ) { return {}; }
        auto const start { c.cbegin() + s };
        auto const subRange { std::ranges::subrange( start ,start + std::ranges::min(e,c.size()) ) };
        return { subRange.cbegin() , subRange.cend() };
    }

    template<typename Con_>
    constexpr auto append(Con_ & c,typename Con_::const_reference v) noexcept -> Con_ &
    { c.push_back(v); return c; }

    template<typename Con_>
    constexpr auto append(Con_ & c,typename Con_::value_type && v) noexcept -> Con_ &
    { c.push_back(std::forward<decltype(v)>(v)); return c; }

    template<typename Con_,typename Con_R >
    requires std::ranges::range<Con_> && std::ranges::input_range<Con_R>
    constexpr auto append(Con_ & fst ,Con_R const & snd) noexcept -> Con_ &
    { fst.insert(fst.cend(),snd.cbegin(),snd.cend()); return fst; }

    template<typename Con_>
    requires std::ranges::range<Con_>
    constexpr auto append(Con_ & c , typename Con_::const_pointer const d,std::size_t const length) noexcept -> Con_ &
    { return append(c,std::ranges::subrange{d, d + length } ); }

    XString trim(XString const & str) noexcept;

    XStringVector split(XString const & str, char delimiter) noexcept;

    template<typename T,typename STR>
    constexpr std::optional<T> toNum(STR const & s,int const base = 10) noexcept {
        T value{};
        return std::from_chars(s.data(),s.data() + s.size(),value,base).ec == std::errc{}
            ? std::optional<T>{value} : std::nullopt;
    }

    template<typename T>
    constexpr std::optional<T> toNum(std::string_view const & s,int const base = 10) noexcept {
        T value{};
        return std::from_chars(s.data(),s.data() + s.size(),value,base).ec == std::errc{}
        ? std::optional<T>{value} : std::nullopt;
    }

    template<typename StringStream,typename T>
    constexpr auto toString(T const v,auto const precision = StringStream{}.precision()) noexcept
        -> decltype(StringStream{}.str()) {
        StringStream ss {};
        ss.precision(precision);
        ss << v;
        return ss.str();
    }

    template<typename T>
    concept StandardChar =
        std::same_as<T, char>
        || std::same_as<T, wchar_t>
        || std::same_as<T, char8_t>
        || std::same_as<T, char16_t>
        || std::same_as<T, char32_t>;

    template<typename T>
    concept WritableCharRange =
        std::ranges::range<T>
        && StandardChar<std::ranges::range_value_t<T>>
        && requires(T & t) { *t.begin() = typename T::value_type{}; };

    template<
        std::ranges::range Range,
        typename Alloc = std::allocator<std::ranges::range_value_t<Range>>
    >
    requires StandardChar<std::ranges::range_value_t<Range>>
    constexpr auto toLower(Range && r, const Alloc & = Alloc{}) noexcept{
        using CharT = std::ranges::range_value_t<Range>;

        if constexpr (WritableCharRange<Range>) {
            // 可写 → 就地修改
            std::ranges::transform(r, r.begin()
                ,[]<typename T0>(T0 const ch) noexcept { return static_cast<T0>(std::tolower(ch)); });
            return std::forward<Range>(r);
        } else {
            // 不可写 → 返回新的 basic_string
            std::basic_string<CharT, std::char_traits<CharT>, Alloc> result{};
            if constexpr (std::ranges::sized_range<Range>)
            { result.reserve(std::ranges::size(r)); }

            std::ranges::transform(r, std::back_inserter(result),
                []<typename T0>(T0 const ch) noexcept { return static_cast<T0>(std::tolower(ch)); });
            return result;
        }
    }

    template<
        std::ranges::range Range,
        typename Alloc = std::allocator<std::ranges::range_value_t<Range>>
    >
    requires StandardChar<std::ranges::range_value_t<Range>>
    constexpr auto toUpper(Range && r, const Alloc & = Alloc{}) noexcept{
        using CharT = std::ranges::range_value_t<Range>;

        if constexpr (WritableCharRange<Range>) {
            // 可写 → 就地修改
            std::ranges::transform(r, r.begin()
                ,[]<typename T0>(T0 const ch) noexcept { return static_cast<T0>(std::toupper(ch)); });
            return std::forward<Range>(r);
        } else {
            // 不可写 → 返回新的 basic_string
            std::basic_string<CharT, std::char_traits<CharT>, Alloc> result{};
            if constexpr (std::ranges::sized_range<Range>)
            { result.reserve(std::ranges::size(r)); }

            std::ranges::transform(r, std::back_inserter(result)
                ,[]<typename T0>(T0 const ch) noexcept { return static_cast<T0>(std::toupper(ch)); });
            return result;
        }
    }
}

#endif
