#include "xabstractthread_p.hpp"
#include <criticalarea.hpp>
#include <xhelper.hpp>
#include <xcontainer.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

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
    if (!m_finished.loadAcquire() || m_running.loadAcquire()) { return; }

    m_finished.storeRelease({});
    m_running.storeRelease(true);

    auto const thName{ (XOStringStream{} << GET_STR(thread:) << m_th_cnt.loadRelaxed() ).str() };

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
        m_threadID.storeRelease({});
        m_finished.storeRelease(true);
        m_running.storeRelease({});
        m_id.storeRelease(-1);
        return;
    }
    m_threadID.storeRelease(thID);
    auto const id{ m_th_cnt.fetchAndAddOrdered(1) };
    m_id.storeRelease(static_cast<int>(id));
}

bool XAbstractThreadPrivate::waitHelper(int64_t const timeOut) noexcept {
    if (m_finished.loadAcquire() || !m_running.loadAcquire()) { return true; }
    auto const pts{ timeOut < 0 ? portMAX_DELAY : static_cast<TickType_t>(timeOut) };
    std::unique_lock lk{m_mtx};
    return m_cv.wait_for(lk,pts,[this]()noexcept
        { return m_finished.loadAcquire() || !m_threadID.loadAcquire() || !m_running.loadAcquire(); });
}

void XAbstractThreadPrivate::finished() noexcept {
    m_threadID.storeRelease({});
    m_finished.storeRelease(true);
    m_id.storeRelease(-1);
    m_running.storeRelease({});
    m_th_cnt.fetchAndSubOrdered(1);
    m_cv.notify_all();
}

void XAbstractThreadPrivate::setPriority(uint32_t const prio) const noexcept {
    auto const thID{m_threadID.loadAcquire()};
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
{ return xPortIsInsideInterrupt() == pdFALSE; }

std::size_t XAbstractThread::threadCount() noexcept
{ return XAbstractThreadPrivate::m_th_cnt.loadAcquire(); }

void * XAbstractThread::threadHandle() const noexcept
{ return d_func()->m_threadID.loadAcquire();}

int XAbstractThread::threadID() const noexcept
{ return d_func()->m_id.loadAcquire(); }

bool XAbstractThread::wait(int64_t const v) noexcept
{ return d_func()->waitHelper(v); }

bool XAbstractThread::isFinished() const noexcept
{ return d_func()->m_finished.loadAcquire(); }

bool XAbstractThread::isRunning() const noexcept
{ return d_func()->m_running.loadAcquire(); }

void XAbstractThread::start(std::size_t const stackSize
                            ,uint32_t const prio,void * const puxStackBuffer,void * const pxTaskBuffer) noexcept
{ d_func()->startHelper(stackSize,prio,puxStackBuffer,pxTaskBuffer); }

void XAbstractThread::setPriority(uint32_t const p) const noexcept
{ d_func()->setPriority(p); }

void XAbstractThread::destroy() noexcept {
    W_D(XAbstractThread);
    if (!d->m_finished.loadAcquire() || d->m_running.loadAcquire()) { return; }
    auto const thID{ d->m_threadID.fetchAndStoreOrdered({}) };
    if (!thID) { return; }
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
    TaskCriticalArea c{};
    if (!d->m_finished.loadAcquire() || d->m_running.loadAcquire() || d->m_threadID.loadAcquire())
    { return; }
    d->m_fn.swap(fn);
}

XAbstractThread::~XAbstractThread()
{ destroy(); }

#endif
