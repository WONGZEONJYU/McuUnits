#include <xtaskbase.hpp>

void XTaskBase::_stop_()
{ Exit(); m_th_.stop();}

void XTaskBase::Strat() {
    if (!m_th_.thread_handle()) {
        m_th_ = XThreadDynamic(2048,&XTaskBase::Main,this);
    }
    m_is_exit_ = false;
    m_th_.start();
}

void XTaskBase::Stop() {_stop_(); }

void XTaskBase::Exit() { _exit_(); }

void XTaskBase::Set_Priority(const uint32_t &p) const noexcept
{ m_th_.setPriority(p); }

XTaskBase::~XTaskBase()
{ m_next_ = nullptr;_stop_(); }
