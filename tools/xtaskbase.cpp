#include <xtaskbase.hpp>

void XTask_base::_stop_()
{ Exit(); m_th_.Stop();}

void XTask_base::Strat() {
    if (!m_th_.thread_handle()){
        m_th_ = XThread_Dynamic(2048,&XTask_base::Main,this);
    }
    m_is_exit_ = false;
    m_th_.Start();
}

void XTask_base::Stop() {_stop_(); }

void XTask_base::Exit() { _exit_(); }

void XTask_base::Set_Priority(const uint32_t &p) const noexcept
{ m_th_.Set_Priority(p); }

XTask_base::~XTask_base()
{ m_next_ = nullptr;_stop_(); }
