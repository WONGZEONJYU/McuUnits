#ifndef STREAM_BUFFER_HPP
#define STREAM_BUFFER_HPP 1

#include <wglobal.hpp>
#include <memory>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <stream_buffer.h>

class XAbstractStreamBuffer {
    W_DISABLE_COPY_MOVE(XAbstractStreamBuffer)
    using StreamBufferHandlePtr = std::unique_ptr<StreamBufferDef_t,decltype(&vStreamBufferDelete)>;
    StreamBufferHandlePtr m_handle_{{},{}};

public:
    virtual ~XAbstractStreamBuffer() = default;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool full() const noexcept;
    [[nodiscard]] size_t bytesAvailable() const noexcept;
    [[nodiscard]] size_t readable() const noexcept;
    [[nodiscard]] size_t spacesAvailable() const noexcept;
    [[nodiscard]] size_t writable() const noexcept;
    [[nodiscard]] bool clean() const noexcept;

    /**
     * 设置字节数量触发等级
     * @param level
     * @return ture or false
     */
    [[nodiscard]] bool setTriggerLevel(size_t level) const noexcept;
    /**
     * 线程发送数据,多个线程同时调用放在一个临界区内才安全,单个线程写可不在临界区内调用
     * 在临界区内调用,timeOut需设置为0(默认参数即可),否则会造成系统无法调度线程导致永远阻塞无法退出
     * @param data
     * @param len
     * @param timeOut 负数表示永久阻塞,0表示不阻塞
     * @return 写入的大小一般小于等于len,-1 表示data,len 参数可能出错了
     */
    int64_t send(const void * data, size_t len,int64_t timeOut = 0) const noexcept;
    /**
     * 中断写入
     * @param data
     * @param len
     * @return 写入的大小一般小于等于len,-1 表示data,len 参数可能出错了
     */
    int64_t sendFromIsr(const void * data, size_t len) const noexcept;
    /**
     * 线程读取数据,多个线程同时调用放在一个临界区内才安全,单个线程写可不在临界区内调用
     * 在临界区内调用,timeOut需设置为0(默认参数即可),否则会造成系统无法调度线程导致永远阻塞无法退出
     * @param data
     * @param len
     * @param timeOut 负数表示永久阻塞,0表示不阻塞
     * @return 读取的大小一般小于等于len,-1 表示data,len 参数可能出错了
     */
    int64_t receive(void * data, size_t len,int64_t timeOut = 0) const noexcept;
    /**
     * 中断读取
     * @param data
     * @param len
     * @return 读取的大小一般小于等于len,-1 表示data,len 参数可能出错了
     */
    int64_t receiveFromIsr(void* data, size_t len) const noexcept;

private:
    constexpr XAbstractStreamBuffer() = default;
    friend class StreamBuffer;
    template<size_t> friend class StaticStreamBuffer;
};

#ifdef configSUPPORT_DYNAMIC_ALLOCATION
#if configSUPPORT_DYNAMIC_ALLOCATION > 0

class StreamBuffer final : public XAbstractStreamBuffer {
    W_DISABLE_COPY_MOVE(StreamBuffer)
public:
    explicit StreamBuffer(size_t = 1024) noexcept;
    ~StreamBuffer() override = default;
};

#endif
#endif

#ifdef configSUPPORT_STATIC_ALLOCATION
#if configSUPPORT_STATIC_ALLOCATION > 0

template<size_t N = 1024>
class StaticStreamBuffer final : public XAbstractStreamBuffer {
    W_DISABLE_COPY_MOVE(StaticStreamBuffer)
    StaticStreamBuffer_t m_ctx_{};
    uint8_t m_buffer_[N]{};

public:
    static constexpr auto max() noexcept{ return N;}
    explicit StaticStreamBuffer() noexcept;
    ~StaticStreamBuffer() override = default;
};

template <size_t N>
StaticStreamBuffer<N>::StaticStreamBuffer() noexcept {
    m_handle_ = StreamBufferHandlePtr {xStreamBufferCreateStatic(max(),1,m_buffer_
        ,std::addressof(m_ctx_)), &vStreamBufferDelete };
}

#endif
#endif
#endif
#endif
