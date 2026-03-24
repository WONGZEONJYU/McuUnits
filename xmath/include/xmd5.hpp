#ifndef FCDETECTOR_XMD5_HPP
#define FCDETECTOR_XMD5_HPP


#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace md5 {

class MD5 {
    uint32_t m_state_[4] {};
    uint64_t m_len_ {};
    uint8_t m_buf_[64]{};
    std::size_t m_buf_len_{};

public:
    constexpr MD5() noexcept { reset(); }

    constexpr void reset() noexcept {
        m_len_ = 0;
        m_buf_len_ = 0;
        m_state_[0] = 0x67452301u;
        m_state_[1] = 0xefcdab89u;
        m_state_[2] = 0x98badcfeu;
        m_state_[3] = 0x10325476u;
    }

    constexpr void update(const void * const data, std::size_t len) noexcept {
        auto p = static_cast<const uint8_t*>(data);
        m_len_ += static_cast<uint64_t>(len) * 8;

        while (len > 0) {
            auto const n{ len < 64 - m_buf_len_ ? len : 64 - m_buf_len_ };
            std::memcpy(m_buf_ + m_buf_len_, p, n);
            m_buf_len_ += n;
            p += n;
            len -= n;

            if (64 == m_buf_len_) {
                transform(m_buf_);
                m_buf_len_ = 0;
            }
        }
    }

    constexpr void finalize() noexcept {
        m_buf_[m_buf_len_++] = 0x80;

        if (m_buf_len_ > 56) {
            while (m_buf_len_ < 64) m_buf_[m_buf_len_++] = 0;
            transform(m_buf_);
            m_buf_len_ = 0;
        }

        while (m_buf_len_ < 56) { m_buf_[m_buf_len_++] = {}; }

        // append length (little endian)
        for (int i = 0; i < 8; ++i)
        { m_buf_[56 + i] = static_cast<uint8_t>(m_len_ >> (8 * i)); }

        transform(m_buf_);
    }

    template<std::size_t Num = 16> requires (16 == Num)
    constexpr void digest(uint8_t (&out)[Num]) const noexcept {
        static_assert(16 == Num,"Num must be 16");
        for (int i {}; i < 4; ++i) {
            out[i * 4 + 0] = static_cast<uint8_t>(m_state_[i]);
            out[i * 4 + 1] = static_cast<uint8_t>(m_state_[i] >> 8);
            out[i * 4 + 2] = static_cast<uint8_t>(m_state_[i] >> 16);
            out[i * 4 + 3] = static_cast<uint8_t>(m_state_[i] >> 24);
        }
    }

    template<std::size_t Num = 16>
    constexpr void operator()(const void * const d,std::size_t const len,uint8_t (&out)[Num] ) noexcept {
        reset();
        update(d,len);
        finalize();
        digest(out);
    }

    template<std::size_t Num = 16>
    static constexpr void compute(const void * const data, std::size_t const len, uint8_t (&out)[Num]) noexcept {
        MD5 ctx{};
        ctx.update(data, len);
        ctx.finalize();
        ctx.digest(out);
    }

private:
    static constexpr uint32_t rotl(uint32_t const x, uint32_t const n) noexcept
    { return x << n | x >> (32 - n); }

    static constexpr uint32_t F(uint32_t const x, uint32_t const y, uint32_t const z) noexcept
    { return (x & y) | (~x & z); }

    static constexpr uint32_t G(uint32_t const x, uint32_t const y, uint32_t const z) noexcept
    { return (x & z) | (y & ~z); }

    static constexpr uint32_t H(uint32_t const x, uint32_t const y, uint32_t const z) noexcept
    { return x ^ y ^ z; }

    static constexpr uint32_t I(uint32_t const x, uint32_t const y, uint32_t const z) noexcept
    { return y ^ (x | ~z); }

    template<std::size_t Num = 64>
    constexpr void transform(const uint8_t (& block)[Num]) noexcept {
        uint32_t a = m_state_[0]
                ,b = m_state_[1]
                ,c = m_state_[2]
                ,d = m_state_[3]
                ,x[16] {};

        for (int i {}; i < 16; ++i) {
            x[i] =
                static_cast<uint32_t>(block[i * 4 + 0]) |
                static_cast<uint32_t>(block[i * 4 + 1]) << 8 |
                static_cast<uint32_t>(block[i * 4 + 2]) << 16 |
                static_cast<uint32_t>(block[i * 4 + 3]) << 24;
        }

#define OP(f,a,b,c,d,x,s,ac) \
    a += f(b,c,d) + x + ac; \
    a = rotl(a, s); \
    a += b;

        OP(F,a,b,c,d,x[ 0], 7,0xd76aa478); OP(F,d,a,b,c,x[ 1],12,0xe8c7b756);
        OP(F,c,d,a,b,x[ 2],17,0x242070db); OP(F,b,c,d,a,x[ 3],22,0xc1bdceee);
        OP(F,a,b,c,d,x[ 4], 7,0xf57c0faf); OP(F,d,a,b,c,x[ 5],12,0x4787c62a);
        OP(F,c,d,a,b,x[ 6],17,0xa8304613); OP(F,b,c,d,a,x[ 7],22,0xfd469501);
        OP(F,a,b,c,d,x[ 8], 7,0x698098d8); OP(F,d,a,b,c,x[ 9],12,0x8b44f7af);
        OP(F,c,d,a,b,x[10],17,0xffff5bb1); OP(F,b,c,d,a,x[11],22,0x895cd7be);
        OP(F,a,b,c,d,x[12], 7,0x6b901122); OP(F,d,a,b,c,x[13],12,0xfd987193);
        OP(F,c,d,a,b,x[14],17,0xa679438e); OP(F,b,c,d,a,x[15],22,0x49b40821);

        OP(G,a,b,c,d,x[ 1], 5,0xf61e2562); OP(G,d,a,b,c,x[ 6], 9,0xc040b340);
        OP(G,c,d,a,b,x[11],14,0x265e5a51); OP(G,b,c,d,a,x[ 0],20,0xe9b6c7aa);
        OP(G,a,b,c,d,x[ 5], 5,0xd62f105d); OP(G,d,a,b,c,x[10], 9,0x02441453);
        OP(G,c,d,a,b,x[15],14,0xd8a1e681); OP(G,b,c,d,a,x[ 4],20,0xe7d3fbc8);
        OP(G,a,b,c,d,x[ 9], 5,0x21e1cde6); OP(G,d,a,b,c,x[14], 9,0xc33707d6);
        OP(G,c,d,a,b,x[ 3],14,0xf4d50d87); OP(G,b,c,d,a,x[ 8],20,0x455a14ed);
        OP(G,a,b,c,d,x[13], 5,0xa9e3e905); OP(G,d,a,b,c,x[ 2], 9,0xfcefa3f8);
        OP(G,c,d,a,b,x[ 7],14,0x676f02d9); OP(G,b,c,d,a,x[12],20,0x8d2a4c8a);

        OP(H,a,b,c,d,x[ 5], 4,0xfffa3942); OP(H,d,a,b,c,x[ 8],11,0x8771f681);
        OP(H,c,d,a,b,x[11],16,0x6d9d6122); OP(H,b,c,d,a,x[14],23,0xfde5380c);
        OP(H,a,b,c,d,x[ 1], 4,0xa4beea44); OP(H,d,a,b,c,x[ 4],11,0x4bdecfa9);
        OP(H,c,d,a,b,x[ 7],16,0xf6bb4b60); OP(H,b,c,d,a,x[10],23,0xbebfbc70);
        OP(H,a,b,c,d,x[13], 4,0x289b7ec6); OP(H,d,a,b,c,x[ 0],11,0xeaa127fa);
        OP(H,c,d,a,b,x[ 3],16,0xd4ef3085); OP(H,b,c,d,a,x[ 6],23,0x04881d05);
        OP(H,a,b,c,d,x[ 9], 4,0xd9d4d039); OP(H,d,a,b,c,x[12],11,0xe6db99e5);
        OP(H,c,d,a,b,x[15],16,0x1fa27cf8); OP(H,b,c,d,a,x[ 2],23,0xc4ac5665);

        OP(I,a,b,c,d,x[ 0], 6,0xf4292244); OP(I,d,a,b,c,x[ 7],10,0x432aff97);
        OP(I,c,d,a,b,x[14],15,0xab9423a7); OP(I,b,c,d,a,x[ 5],21,0xfc93a039);
        OP(I,a,b,c,d,x[12], 6,0x655b59c3); OP(I,d,a,b,c,x[ 3],10,0x8f0ccc92);
        OP(I,c,d,a,b,x[10],15,0xffeff47d); OP(I,b,c,d,a,x[ 1],21,0x85845dd1);
        OP(I,a,b,c,d,x[ 8], 6,0x6fa87e4f); OP(I,d,a,b,c,x[15],10,0xfe2ce6e0);
        OP(I,c,d,a,b,x[ 6],15,0xa3014314); OP(I,b,c,d,a,x[13],21,0x4e0811a1);
        OP(I,a,b,c,d,x[ 4], 6,0xf7537e82); OP(I,d,a,b,c,x[11],10,0xbd3af235);
        OP(I,c,d,a,b,x[ 2],15,0x2ad7d2bb); OP(I,b,c,d,a,x[ 9],21,0xeb86d391);

#undef OP
        m_state_[0] += a;
        m_state_[1] += b;
        m_state_[2] += c;
        m_state_[3] += d;
    }
};

}

#endif
