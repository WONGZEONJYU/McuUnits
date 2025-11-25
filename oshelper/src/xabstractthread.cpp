#include "xabstractthread_p.hpp"
#include <criticalarea.hpp>
#include <xhelper.hpp>
#include <xcontainer.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

void XAbstractThreadPrivate::start(void * const parm) {
    auto const thr{ static_cast<XAbstractThreadPrivate*>(parm) };
    if (auto const & fn{ thr->m_fn })
    { fn->operator()(); }
    thr->finished();
    vTaskDelete(nullptr);
}

void XAbstractThreadPrivate::startHelper(std::size_t const uxStackDepth,
                                        uint32_t const prio
                                        ,[[maybe_unused]] void * const puxStackBuffer
                                        ,[[maybe_unused]] void * const pxTaskBuffer ) noexcept
{
    if (!m_finished.loadRelaxed()) { return; }

    m_finished.storeRelaxed({});

    auto const thName{ (XOStringStream{} << GET_STR(thread:) << m_th_cnt.loadRelaxed() ).str() };

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
        m_threadID.storeRelaxed({});
        m_finished.storeRelease(true);
        return;
    }
    m_threadID.storeRelease(thID);
    auto const id{ m_th_cnt.fetchAndAddOrdered(1) };
    m_id.storeRelease(static_cast<int>(id));
}

bool XAbstractThreadPrivate::waitHelper(int64_t const timeOut) noexcept {
    if (m_finished.loadRelaxed()) { return true; }
    auto const pts{ timeOut < 0 ? portMAX_DELAY : static_cast<TickType_t>(timeOut) };
    std::unique_lock lk{m_mtx};
    return m_cv.wait_for(lk,pts,[this]()noexcept{ return m_finished.loadRelaxed();});
}

void XAbstractThreadPrivate::finished() noexcept {
    m_threadID.storeRelaxed({});
    m_finished.storeRelaxed(true);
    m_th_cnt.fetchAndSubOrdered(1);
    m_cv.notify_all();
}

void XAbstractThreadPrivate::setPriority(uint32_t const prio) const noexcept {
    CHECK_EMPTY(m_threadID.loadRelaxed(),return);
    vTaskPrioritySet(static_cast<TaskHandle_t>(m_threadID.loadRelaxed()),prio);
}

#if 0
void XAbstractThread::taskReturn() noexcept
{ vTaskDelete(nullptr); }
#endif

void XAbstractThread::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XAbstractThread::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

void XAbstractThread::yield() noexcept
{ taskYIELD();}

bool XAbstractThread::isRunningInThread() noexcept
{ return !static_cast<bool>(xPortIsInsideInterrupt()); }

std::size_t XAbstractThread::thread_count() noexcept {
#if 0
    return m_th_cnt_.loadAcquire();
#endif
    return XAbstractThreadPrivate::m_th_cnt.loadRelaxed();
}

void * XAbstractThread::thread_handle() const noexcept {
#if 0
    return m_thread_.loadAcquire();
#endif
    return d_func()->m_threadID.loadRelaxed();
}

int XAbstractThread::thread_id() const noexcept {
#if 0
    return m_id_.loadAcquire();
#endif
    return d_func()->m_id.loadRelaxed();
}

bool XAbstractThread::wait(int64_t const v) noexcept
{ return d_func()->waitHelper(v); }

#if 0

#define TH auto const th { thread_handle() }

void XAbstractThread::start() const noexcept {
    TH;CHECK_EMPTY(th,return); vTaskResume(static_cast<TaskHandle_t>(th));
}

void XAbstractThread::start() const noexcept {
    TH;CHECK_EMPTY(th,return); vTaskResume(static_cast<TaskHandle_t>(th));
}

void XAbstractThread::stop() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskSuspend(static_cast<TaskHandle_t>(th)); }

#endif

void XAbstractThread::start(std::size_t const stackSize
    ,uint32_t const prio,void * const puxStackBuffer,void * const pxTaskBuffer) noexcept {
    d_func()->startHelper(stackSize,prio,puxStackBuffer,pxTaskBuffer);
}

void XAbstractThread::setPriority(uint32_t const p) const noexcept {
#if 0
    TH;CHECK_EMPTY(th,return); vTaskPrioritySet(static_cast<TaskHandle_t>(thread_handle()),p);
#endif
    d_func()->setPriority(p);
}

void XAbstractThread::destroy() noexcept {
#if 0
    TH;CHECK_EMPTY(th,return);
    vTaskSuspend(static_cast<TaskHandle_t>(th));
    vTaskDelete(static_cast<TaskHandle_t>(th));
    m_thread_.storeRelease({});
#endif

    W_D(XAbstractThread);
    CHECK_ERR(!d->m_finished.loadRelaxed(),return);
    auto const thID{ d->m_threadID.loadRelaxed() };
    CHECK_EMPTY(!thID,return);
    d->m_threadID.storeRelaxed({});
    vTaskDelete(static_cast<TaskHandle_t>(thID));
}

XAbstractThread::XAbstractThread()
    :m_d_ptr_ { makeUnique<XAbstractThreadPrivate>().release() }
{
    if (m_d_ptr_) { return; }
    m_d_ptr_->q_ptr = this;
}

XAbstractThread::XAbstractThread(CallablePtr fn)
    : XAbstractThread()
{
    W_D(XAbstractThread);
    if (!d) { return; }
    d_func()->m_fn.swap(fn);
}

void XAbstractThread::setThreadFn(CallablePtr fn) {
    W_D(XAbstractThread);
    CHECK_ERR(!d->m_finished.loadRelaxed(),return);
    CHECK_ERR(!d->m_threadID.loadRelaxed(),return);
    d->m_fn.swap(fn);
}

XAbstractThread::~XAbstractThread()
{ destroy(); }

#if 0
void XAbstractThread::setInfo(void * const th) noexcept {
    if (th) {
        auto const th_{ static_cast<TaskHandle_t>(th) };
        m_thread_.storeRelease(th_);
        auto const id { m_th_cnt_.fetchAndAddOrdered(1) };
        m_id_.storeRelease(static_cast<int>(id));
        vTaskSuspend(th_);
    }
}
#endif

#if 0
void XAbstractThread::swap(XAbstractThread & o) noexcept {

    TaskCriticalArea c;
    auto const id { m_id_.loadAcquire() };
    auto const th{ m_thread_.loadAcquire() };
    m_id_.storeRelease(o.m_id_.loadAcquire());
    m_thread_.storeRelease(o.m_thread_.loadAcquire());
    o.m_id_.storeRelease(id);
    o.m_thread_.storeRelease(th);

}
#endif

#if 0

void XAbstractThread::createTask(void(*f)(void*)
                                , std::size_t const uxStackDepth
                                , void * const pvParameters
                                , void * const puxStackBuffer
                                , void * const pxTaskBuffer) noexcept
{
    XStringStream ss {};
    ss << GET_STR(thread:) << m_th_cnt_.loadRelaxed();
    TaskCriticalArea c{};
#if configSUPPORT_STATIC_ALLOCATION > 0
    if (puxStackBuffer && pxTaskBuffer) {
        auto const th{ xTaskCreateStatic(f,ss.str().c_str(),uxStackDepth,pvParameters,configMAX_PRIORITIES
            ,static_cast<StackType_t*>(puxStackBuffer),static_cast<StaticTask_t*>(pxTaskBuffer)) };
        setInfo(th);
        return;
    }
#endif

#if configSUPPORT_DYNAMIC_ALLOCATION > 0
    tskTaskControlBlock * th{};
    xTaskCreate(f,ss.str().c_str(),uxStackDepth,pvParameters,configMAX_PRIORITIES,&th);
    setInfo(th);
#endif
}

#endif

#endif
