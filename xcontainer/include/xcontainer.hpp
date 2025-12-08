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
#include <array>
#include <sstream>
#include <string>
#include <charconv>
#include <xtypetraits.hpp>

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
            auto const canWrite{ std::ranges::min(static_cast<int64_t>(s),writableSize()) };
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
            auto const canRead{ std::ranges::min(static_cast<int64_t>(s),readableSize()) };
            int64_t c {};
            for (;c < canRead;++c) { if (!read(d[c])){ break; } }
            return c;
        }

        constexpr int64_t peek(value_type * const d,std::size_t const s) const noexcept {
            if (!d) { return -1; }
            if (empty()) { return 0; }
            auto const canRead{ std::ranges::min(static_cast<int64_t>(s),readableSize()) };
            int64_t c {};
            for (auto pos{ m_r_.loadAcquire() % Size_ };c < canRead;++c,++pos)
                { d[c] = m_buf_[pos]; }
            return c;
        }
    };

    template<typename> class RingBufferIterator;

    template<typename ,std::size_t N = 1024> requires(N > 0) class RingBuffer;

    template<typename T,std::size_t N> requires(N > 0)
    class RingBuffer {
        static_assert(N > 0,"RingBuffer Size must be greater than 0");
        static_assert(!std::is_const_v<T>, "RingBuffer does not support const types");
        template<typename> friend class RingBufferIterator;

        std::array<T,N> m_buffer_{};
        XAtomicInteger<std::size_t> m_head_{},m_tail_{},m_size_{};

    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type*;
        using const_pointer = const value_type *;
        using iterator = RingBufferIterator<RingBuffer>;
        using const_iterator = RingBufferIterator<RingBuffer const>;

        constexpr RingBuffer() = default;

        constexpr explicit RingBuffer(value_type const (&values)[N])
            : m_tail_{N - 1}, m_size_{N}
        { std::ranges::copy(std::begin(values), std::end(values),m_buffer_.begin()); }

        constexpr explicit RingBuffer(const_reference v)
            :m_tail_{N - 1}, m_size_{N}
        { std::ranges::fill(m_buffer_.begin(), m_buffer_.end(),v); }

        constexpr RingBuffer(RingBuffer const &) = default;
        RingBuffer &operator=(RingBuffer const &) = default;

        constexpr RingBuffer(RingBuffer &&) = default;
        RingBuffer &operator=(RingBuffer &&) = default;

        constexpr auto size() const noexcept{ return m_size_.loadRelaxed(); }
        static constexpr auto capacity() noexcept{ return N; }
        constexpr auto empty() const noexcept{ return !size(); }
        constexpr auto full() const noexcept{ return N == size(); }

        constexpr void clear() noexcept {
            m_size_.storeRelease({});
            m_head_.storeRelease({});
            m_tail_.storeRelease({});
        }

        constexpr reference operator[](size_type const pos) noexcept
        { return m_buffer_[(m_head_.loadAcquire() + pos) % N]; }

        constexpr const_reference operator[](size_type const pos) const noexcept
        { return const_cast<RingBuffer&>(*this)[pos]; }

        constexpr reference at(size_type const pos) {
            if (pos < size()) { m_buffer_[(m_head_.loadAcquire() + pos) % N];}
            abort();
        }

        constexpr const_reference at(size_type const pos) const
        { return const_cast<RingBuffer*>(this)->at(pos); }

        constexpr reference front() {
            if(size() > 0) { return m_buffer_[m_head_.loadAcquire()]; }
            abort();
        }

        constexpr const_reference front() const
        { return const_cast<RingBuffer*>(this)->front(); }

        constexpr reference back() {
            if(size() > 0) {return m_buffer_[m_tail_.loadAcquire()];}
            abort();
        }

        constexpr const_reference back() const
        { return const_cast<RingBuffer*>(this)->back(); }

        constexpr void push_back(value_type const & value)
        { append(value); }

        constexpr void push_back(value_type && value)
        { append(std::move(value)); }

        constexpr value_type pop_front() {
            if (empty()) { abort(); }
            auto const index{ m_head_.loadAcquire() };
            auto value{ std::move(m_buffer_[index]) };
            m_head_.storeRelease((index + 1) % N);
            m_size_.deref();
            return value;
        }

        iterator begin() { return iterator(this, 0); }
        iterator end() { return iterator(this, size()); }

        const_iterator begin() const { return const_iterator(this, 0); }
        const_iterator end() const { return const_iterator(this, size()); }

        const_iterator cBegin() const { return const_iterator(this, 0); }
        const_iterator cEnd() const { return const_iterator(this, size()); }

    private:
        template<typename Tp> requires std::is_same_v<std::decay_t<Tp>,value_type>
        constexpr void append(Tp && value) {
            if(empty()) {
                m_size_.ref();
            } else if(!full()) {
                m_tail_.storeRelease((m_tail_.loadAcquire() + 1) % N);
                m_size_.ref();
            } else {
                m_head_.storeRelease((m_head_.loadAcquire() + 1) % N);
                m_tail_.storeRelease((m_tail_.loadAcquire() + 1) % N);
            }
            m_buffer_[m_tail_.loadAcquire()] = std::forward<Tp>(value);
        }
    };

    template<typename RingBufferType>
    class RingBufferIterator {

    template<typename ,std::size_t N> requires(N > 0) friend class RingBuffer;

    using ringBuffer_type = RingBufferType;

    using RefRingBuffer = ringBuffer_type *;

    RefRingBuffer m_ref_{};
    RingBufferType::size_type m_index_{};

    public:
        using iterator_category = std::random_access_iterator_tag;
        using self_type = RingBufferIterator;
        using value_type = RingBufferType::value_type;
        using size_type = RingBufferType::size_type;
        using difference_type = RingBufferType::difference_type;
        using reference = std::conditional_t< std::is_const_v<ringBuffer_type>
                ,typename RingBufferType::const_reference
                ,typename RingBufferType::reference
            >;
        using pointer = std::conditional_t<std::is_const_v<ringBuffer_type>
                    ,typename RingBufferType::const_pointer
                    , typename RingBufferType::pointer
            >;

    constexpr RingBufferIterator() = default;

    template<typename OtherIter>
    constexpr RingBufferIterator(RingBufferIterator<OtherIter> const & other)
        :m_ref_ { other.m_ref_ },m_index_ { other.m_index_ } {}

    constexpr self_type & operator++() {
        if (m_index_ >= m_ref_->size()) { abort();}
        ++m_index_;
        return *this;
    }

    constexpr self_type operator++(int)
    { auto temp{*this}; ++*this; /*operator++();*/ return temp; }

    constexpr self_type & operator--() {
        if (m_index_ <= 0) { abort();}
        --m_index_;
        return *this;
    }

    constexpr self_type operator--(int)
    { auto temp{*this}; --*this; /*operator--(); */ return temp; }

    constexpr self_type operator+(difference_type const offset) const
    { auto temp{*this}; return temp += offset ; /* return temp.operator+=(offset); */ }

    constexpr self_type operator-(difference_type const offset) const
    { auto temp{ *this }; return temp -= offset; /*return temp.operator-=(offset);*/ }

    constexpr difference_type operator-(self_type const & other) const
    { return m_index_ - other.m_index_; }

    constexpr self_type & operator+=(difference_type const offset) {
        auto const next { (m_index_ + offset) % ringBuffer_type::capacity() };
        if (next >= m_ref_->size()) { abort(); }
        m_index_ = next;
        return *this;
    }

    constexpr self_type & operator-=(difference_type const offset)
    { return *this += -offset; /* return operator+=(-offset); */ }

    constexpr reference operator[](difference_type const offset) const
    { return *(*this + offset); /* return operator+(offset).operator*(); */ }

    constexpr reference operator*() const {
        if (m_ref_->empty() || !inBounds()) { abort(); }
        return m_ref_->m_buffer_[(m_ref_->m_head_.loadAcquire() + m_index_) % RingBufferType::capacity()];
    }

    constexpr pointer operator->() const
    { return std::addressof(operator*()); }

    constexpr bool compatible(self_type const & other) const noexcept
    { return dPtr() == other.dPtr(); }

    [[nodiscard]] constexpr bool inBounds() const noexcept {
        return !m_ref_->empty()
            && (m_ref_->m_head_.loadAcquire() + m_index_) % RingBufferType::capacity()
            <= m_ref_->m_tail_.loadAcquire();
    }

    bool operator<(self_type const & other) const noexcept
    { return m_index_ < other.m_index_; }

    bool operator>(self_type const & other) const noexcept
    { return other < *this; }

    bool operator<=(self_type const & other) const noexcept
    { return !(other < *this); }

    bool operator>=(self_type const & other) const noexcept
    { return !(*this < other); }

    template<typename L,typename R>
    friend constexpr bool operator==(RingBufferIterator<L> const & ,RingBufferIterator<R> const & ) noexcept;

    template<typename L,typename R>
    friend constexpr bool operator!=(RingBufferIterator<L> const & ,RingBufferIterator<R> const & ) noexcept;

    template<typename L,typename R>
    friend constexpr std::strong_ordering operator<=> (RingBufferIterator<L> const & ,RingBufferIterator<R> const & ) noexcept;

    private:
        auto dPtr() const noexcept
        { return m_ref_->m_buffer_.data(); }

        RingBufferIterator(ringBuffer_type * const rb, size_type const index)
        : m_ref_{rb}, m_index_{index} {}
    };

    template<typename L,typename R>
    constexpr bool operator==(RingBufferIterator<L> const & x,RingBufferIterator<R> const& y) noexcept
    { return x.dPtr() == y.dPtr() && x.m_index_ == y.m_index_; }

    template<typename L,typename R>
    constexpr bool operator!=(RingBufferIterator<L> const & x,RingBufferIterator<R> const& y) noexcept
    { return !(x == y); }

    template<typename L,typename R>
    constexpr std::strong_ordering operator<=> (RingBufferIterator<L> const & x,RingBufferIterator<R> const & y) noexcept {
        if (auto const cmp {x.dPtr() <=> y.dPtr()}; 0 != cmp) { return cmp; }
        return x.m_index_ <=> y.m_index_;
    }

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

    template<typename Con_,typename Con_R >
    requires std::ranges::range<Con_> && std::ranges::input_range<Con_R>
    constexpr auto append(Con_ & c ,Con_R const & snd) noexcept -> Con_ &
    { c.insert(c.cend(), snd.begin(), snd.end());return c; }

    template<typename Con_>
    requires std::ranges::range<Con_>
    constexpr auto append(Con_ & c , typename Con_::const_pointer d, std::size_t const length)
    noexcept -> Con_ & { return append(c,std::ranges::subrange{d, d + length}); }

    template<typename Con_, typename ...Args>
    requires (
        std::ranges::range<Con_>
        // 禁止 append(c, il) 落入 Args... 重载
        && !(sizeof...(Args) == 1 && is_initializer_list_v<std::remove_cvref_t<std::tuple_element_t<0, std::tuple<Args...>>>>)
        && (std::is_constructible_v<typename Con_::value_type, Args&&> && ...)
    )
    constexpr auto append(Con_ & c, Args && ...args) noexcept -> Con_ &
    { (c.push_back(std::forward<Args>(args)), ...); return c; }

    template<typename Con_>
    requires std::ranges::range<Con_>
    constexpr auto append(Con_ & c, std::initializer_list<typename Con_::value_type> il)
    noexcept -> Con_ & { return append(c,std::ranges::subrange{il.begin(), il.end()}); }

    XString trim(XString const & str) noexcept;

    XStringVector split(XString const & str, char delimiter) noexcept;

    template<typename T,typename STR>
    requires std::is_arithmetic_v<T> && std::is_integral_v<T>
    constexpr auto toNum(STR && s,int const base /* = 10 */)
        noexcept ->std::optional<T>
    {
        T value{};
        return std::from_chars(s.data(),s.data() + s.size(),value,base).ec == std::errc{}
            ? std::optional<T>{value} : std::nullopt;
    }

    template<typename T,typename STR>
    requires std::is_arithmetic_v<T> && std::is_floating_point_v<T>
    constexpr auto toNum(STR && s,std::chars_format const fmt = std::chars_format::general)
        noexcept -> std::optional<T>
    {
        T value{};
        return std::from_chars(s.data(),s.data() + s.size(),value,fmt).ec == std::errc{}
            ? std::optional<T>{value} : std::nullopt;
    }

    template<typename T,typename SS = XStringStream,typename STR>
    constexpr auto toNum(STR && s) noexcept -> std::optional<T>
    { SS ss {}; ss << s; T value{}; return ss >> value ? std::optional<T>{value} : std::nullopt; }

    template<typename SS = XStringStream,typename T>
    constexpr auto toString(T && v,decltype(SS{}.precision()) const precision = SS{}.precision())
        noexcept -> decltype(SS{}.str())
    { SS ss {};ss.precision(precision);ss << v;return ss.str(); }

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
