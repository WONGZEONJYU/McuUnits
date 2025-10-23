#include <streambuffer.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)

bool XAbstractStreamBuffer::empty() const noexcept
{ return xStreamBufferIsEmpty(m_handle_.get()); }

bool XAbstractStreamBuffer::full() const noexcept
{ return xStreamBufferIsFull(m_handle_.get()); }

size_t XAbstractStreamBuffer::bytesAvailable() const noexcept
{ return xStreamBufferBytesAvailable(m_handle_.get()); }

size_t XAbstractStreamBuffer::readable() const noexcept
{ return xStreamBufferBytesAvailable(m_handle_.get()); }

size_t XAbstractStreamBuffer::spacesAvailable() const noexcept
{ return xStreamBufferSpacesAvailable(m_handle_.get()); }

size_t XAbstractStreamBuffer::writable() const noexcept
{ return xStreamBufferSpacesAvailable(m_handle_.get()); }

bool XAbstractStreamBuffer::clean() const noexcept
{ return xStreamBufferReset(m_handle_.get()); }

bool XAbstractStreamBuffer::setTriggerLevel(size_t const level) const noexcept
{ return xStreamBufferSetTriggerLevel(m_handle_.get(),level); }

int64_t XAbstractStreamBuffer::send(const void * const data, size_t const len, int64_t const timeOut) const noexcept {
    if (!data || len <= 0) { return -1; }
    return xStreamBufferSend(m_handle_.get(),data,len,timeOut < 0 ? portMAX_DELAY : timeOut);
}

int64_t XAbstractStreamBuffer::sendFromIsr(const void * const data, size_t const len) const noexcept {
    if (!data || !len ) { return -1; }
    BaseType_t x{};
    auto const ret{ xStreamBufferSendFromISR(m_handle_.get(),data,len,std::addressof(x)) };
    portYIELD_FROM_ISR(x);
    return ret;
}

int64_t XAbstractStreamBuffer::receive(void * const data, size_t const len, int64_t const timeOut) const noexcept {
    if (!data || !len) { return -1; }
    return xStreamBufferReceive(m_handle_.get(),data,len,timeOut < 0 ? portMAX_DELAY : timeOut);
}

int64_t XAbstractStreamBuffer::receiveFromIsr(void * const data, size_t const len) const noexcept {
    if (!data || !len) { return -1; }
    BaseType_t x{};
    auto const ret{ xStreamBufferReceiveFromISR(m_handle_.get(),data,len,std::addressof(x)) };
    portYIELD_FROM_ISR(x);
    return ret;
}

StreamBuffer::StreamBuffer(size_t const len) noexcept
{ m_handle_ = StreamBufferHandlePtr {xStreamBufferCreate(len,1) ,&vStreamBufferDelete}; }

#endif
