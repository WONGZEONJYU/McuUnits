#ifndef X_CONTAINER_HPP
#define X_CONTAINER_HPP 1

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

inline namespace XContainer {

    template<typename T,std::size_t Size_>
    class CircularBuffer final {

        X_DISABLE_COPY(CircularBuffer)

        static_assert(Size_ > 0,"Size_ is less than or equal zero!");

        mutable std::array<T,Size_> m_buf_{};

        mutable std::atomic_uint m_w_{},m_r_{};

    public:
        using value_type = T;

        explicit CircularBuffer() = default;

        CircularBuffer(CircularBuffer && obj) noexcept
        { swap(obj); }

        CircularBuffer& operator=(CircularBuffer &&obj) noexcept
        { CircularBuffer(obj).swap(*this); return *this; }

        void swap(CircularBuffer const & obj) noexcept {
            m_buf_.swap(obj.m_buf_);
            m_w_ = obj.m_w_.exchange({});
            m_r_ = obj.m_r_.exchange({});
        }

        static auto capacity() noexcept
        { return Size_; }

        auto empty() const noexcept
        { return m_w_.load(std::memory_order_relaxed) == m_r_.load(std::memory_order_relaxed); }

        auto full() const noexcept
        { return Size_ == m_w_.load(std::memory_order_relaxed) - m_r_.load(std::memory_order_relaxed); }

        [[nodiscard]] int64_t readableSize() const noexcept {
            if (empty()) { return {}; }
            if (m_w_.load(std::memory_order_relaxed) > m_r_.load(std::memory_order_relaxed))
                { return m_w_.load(std::memory_order_relaxed) - m_r_.load(std::memory_order_relaxed); }
            return Size_ - m_r_.load(std::memory_order_relaxed) + m_w_.load(std::memory_order_relaxed);
        }

        [[nodiscard]] int64_t writableSize() const noexcept {
            if (full()) { return int{}; }
            return Size_ - readableSize();
        }

        void clear() const noexcept {
            m_r_.store(0,std::memory_order_relaxed);
            m_w_.store(0,std::memory_order_relaxed);
        }

        bool write(value_type const & d) const noexcept {
            if (full()) { return {}; }
            m_buf_[m_w_.load(std::memory_order_relaxed) % Size_] = d;
            m_w_.fetch_add(1,std::memory_order_relaxed);
            return true;
        }

        bool write(value_type && d) const noexcept {
            if (full()) { return {}; }
            m_buf_[m_w_.load(std::memory_order_relaxed) % Size_] = std::move(d);
            m_w_.fetch_add(1,std::memory_order_relaxed);
            return true;
        }

        int64_t write(const value_type * const d,std::size_t const s) const noexcept {
            CHECK_EMPTY(d,return -1);
            if (full()) { return 0;}
            auto const canWrite{ std::min(static_cast<int64_t>(s),writableSize()) };
            int64_t c {};
            for (;c < canWrite;++c) { if (!write(d[c])) { break; } }
            return c;
        }

        bool read(value_type & d) const noexcept {
            if (empty()) { return {}; }
            d = m_buf_[m_r_.load(std::memory_order_relaxed) % Size_];
            m_r_.fetch_add(1,std::memory_order_relaxed);
            return true;
        }

        int64_t read(value_type * const d,std::size_t const s) const noexcept {
            CHECK_EMPTY(d,return -1);
            if (empty()) { return 0; }
            auto const canRead{ std::min(static_cast<int64_t>(s),readableSize()) };
            int64_t c {};
            for (;c < canRead;++c) { if (!read(d[c])){ break; } }
            return c;
        }
    };

    template<typename Tp_>
    using XVector = std::vector< Tp_, XAllocator< Tp_ > >;

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

}

#endif
