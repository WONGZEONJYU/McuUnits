#include <xtaskbase.hpp>

void XTaskBase::stop_() noexcept
{ exit(); m_th_.stop();}

void XTaskBase::exit_() noexcept
{ m_isRunning_.storeRelease({}); }

void XTaskBase::start(std::size_t const stack_depth) noexcept {
    if (!m_th_.thread_handle())
    { m_th_ = XThreadDynamic(stack_depth,&XTaskBase::run,this); }
    m_isRunning_.storeRelease(true);
    m_th_.start();
}

bool XTaskBase::isRunning() const noexcept
{ return m_isRunning_.loadRelaxed(); }

void XTaskBase::stop() noexcept
{ stop_(); }

void XTaskBase::exit() noexcept
{ exit_(); }

void XTaskBase::setPriority(uint32_t const p) const noexcept
{ m_th_.setPriority(p); }

XTaskBase::~XTaskBase()
{ stop_(); }
