#include <xthread.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#if configSUPPORT_DYNAMIC_ALLOCATION > 0

XThreadDynamic::~XThreadDynamic()
{ wait(); }

XThreadDynamic::XThreadDynamic(XThreadDynamic && o) noexcept
{ m_d_ptr_.swap(o.m_d_ptr_); }

XThreadDynamic & XThreadDynamic::operator=(XThreadDynamic && o) noexcept
{ if (this != std::addressof(o)) { wait(); swap(o);} return *this; }

void XThreadDynamic::swap(XThreadDynamic & o) noexcept
{ m_d_ptr_.swap(o.m_d_ptr_); }

void XThreadDynamic::start(std::size_t const stackSize, uint32_t const prio) noexcept
{ if (!isRunningInThread()) { return; } XAbstractThread::start(stackSize, prio, {}, {}); }

#endif
#endif
