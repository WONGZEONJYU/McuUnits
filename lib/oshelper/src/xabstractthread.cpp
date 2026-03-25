#include <xabstractthread_p.hpp>
#include <criticalarea.hpp>
#include <xcontainer.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <FreeRTOS.h>
#include <task.h>
#include <atomic.h>
extern "C"{
#include <cmsis_compiler.h>
}

void XAbstractThreadPrivate::start(void * const args) noexcept {
    auto const thr{ static_cast<XAbstractThreadPrivate*>(args) };
    if (auto const fn{ thr->m_fn.get() })
    { (*fn)(); }
    thr->finished();
    vTaskDelete(nullptr);
}

void XAbstractThreadPrivate::startHelper(std::size_t const uxStackDepth,
                                        uint32_t prio
                                        ,[[maybe_unused]] void * const puxStackBuffer
                                        ,[[maybe_unused]] void * const pxTaskBuffer ) noexcept
{
    if (!m_finished || m_running) { return; }

    m_finished = {};
    m_running = true;

    auto const thName{ (XOStringStream{} << GET_STR(thread:) << m_th_cnt ).str() };

    if (prio >= configMAX_PRIORITIES) { prio = configMAX_PRIORITIES - 1;}

    TaskCriticalArea c{};

#if configSUPPORT_STATIC_ALLOCATION > 0
    if (puxStackBuffer && pxTaskBuffer) {
        auto const th{ xTaskCreateStatic(start,thName.c_str(),uxStackDepth,this,prio
            ,static_cast<StackType_t*>(puxStackBuffer),static_cast<StaticTask_t*>(pxTaskBuffer)) };
        setInfo(th);
        return;
    }
#endif

#if configSUPPORT_DYNAMIC_ALLOCATION > 0
    TaskHandle_t th{};
    xTaskCreate(start,thName.c_str(),uxStackDepth,this,prio,std::addressof(th));
    setInfo(th);
#endif
}

void XAbstractThreadPrivate::setInfo(void * const thID) noexcept {
    if (!thID) {
        m_threadID = {};
        m_finished = true;
        m_running = {};
        m_id = -1;
        return;
    }
    m_threadID = thID;
    auto const id{ Atomic_Increment_u32(std::addressof(m_th_cnt)) };
    m_id = static_cast<int>(id);
}

bool XAbstractThreadPrivate::waitHelper(int64_t const timeOut) noexcept {

    if (m_finished || m_detached || !m_running) { return true; }

    std::unique_lock lk{m_mtx};
    auto const pts{ timeOut < 0 ? portMAX_DELAY : static_cast<TickType_t>(timeOut) };
    return m_cv.wait_for(lk,pts,[this]()noexcept
        { return m_finished || !m_threadID || !m_running; });
}

void XAbstractThreadPrivate::finished() noexcept {
    m_threadID = {};
    m_finished = true;
    m_id = -1;
    m_running = {};
    Atomic_Decrement_u32(std::addressof(m_th_cnt));
    if (m_detached) { return; }
    m_cv.notify_all();
}

void XAbstractThreadPrivate::setPriority(uint32_t const prio) const noexcept {
    auto const thID{ m_threadID };
    CHECK_ERR(thID,return);
    vTaskPrioritySet(static_cast<TaskHandle_t>(thID),prio);
}

void XAbstractThread::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XAbstractThread::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

void XAbstractThread::yield() noexcept
{ taskYIELD();}

bool XAbstractThread::isRunningInThread() noexcept
{ return !__get_IPSR(); }

std::size_t XAbstractThread::threadCount() noexcept
{ return XAbstractThreadPrivate::m_th_cnt; }

void * XAbstractThread::threadHandle() const noexcept
{ return d_func()->m_threadID;}

int XAbstractThread::threadID() const noexcept
{ return d_func()->m_id; }

bool XAbstractThread::wait(int64_t const v) noexcept
{ return d_func()->waitHelper(v); }

void XAbstractThread::detach() noexcept
{ d_func()->m_detached = true; }

bool XAbstractThread::isFinished() const noexcept
{ return d_func()->m_finished; }

bool XAbstractThread::isRunning() const noexcept
{ return d_func()->m_running; }

void XAbstractThread::start(std::size_t const stackSize
                            ,uint32_t const prio,void * const puxStackBuffer,void * const pxTaskBuffer) noexcept
{ d_func()->startHelper(stackSize,prio,puxStackBuffer,pxTaskBuffer); }

void XAbstractThread::setPriority(uint32_t const p) const noexcept
{ d_func()->setPriority(p); }

void XAbstractThread::destroy() noexcept {
    W_D(XAbstractThread);
    if (!d->m_finished || d->m_running) { return; }
    auto const thID{ d->m_threadID };
    if (!thID) { return; }
    d->m_threadID = {};
    vTaskDelete(static_cast<TaskHandle_t>(thID));
}

XAbstractThread::XAbstractThread()
    :m_d_ptr_ { makeUnique<XAbstractThreadPrivate>().release() }
{ if (m_d_ptr_) { m_d_ptr_->q_ptr = this; } }

XAbstractThread::XAbstractThread(CallablePtr fn)
: XAbstractThread()
{ W_D(XAbstractThread); if (d) { d->m_fn.swap(fn); } }

void XAbstractThread::setThreadFn(CallablePtr fn) {
    W_D(XAbstractThread);
    CHECK_EMPTY(d,return);
    if (!d->m_finished || d->m_running || d->m_threadID)
    { return; }
    d->m_fn.swap(fn);
}

XAbstractThread::~XAbstractThread()
{ destroy(); }

#endif
