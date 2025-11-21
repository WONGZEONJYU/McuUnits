#include <xthread.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#if configSUPPORT_DYNAMIC_ALLOCATION > 0

XThreadDynamic::XThreadDynamic(XThreadDynamic && o) noexcept
{ XAbstractThread::swap(o);}

XThreadDynamic & XThreadDynamic::operator=(XThreadDynamic && o) noexcept
{ XThreadDynamic{std::move(o)}.swap(*this); return *this; }

#endif
#endif
