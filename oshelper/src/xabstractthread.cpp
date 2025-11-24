#include "xabstractthread_p.hpp"
#include <criticalarea.hpp>
#include <xhelper.hpp>
#include <xcontainer.hpp>

#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

void XAbstractThreadPrivate::start(void * const parm) {
    auto const thr{ static_cast<XAbstractThreadPrivate*>(parm) };
    std::unique_lock lk{thr->m_mtx};
    thr->m_finished.storeRelaxed({});
    if (auto const & fn{ thr->m_fn })
    { fn->operator()(); }
    thr->m_finished.storeRelaxed(true);
    thr->m_thread.storeRelease({});
    vTaskDelete(nullptr);
}

void XAbstractThreadPrivate::startHelper(std::size_t) noexcept {

    if (!m_finished.loadRelaxed()) { return; }

}

void XAbstractThreadPrivate::finished() noexcept
{

}

void XAbstractThread::taskReturn() noexcept
{ vTaskDelete(nullptr); }

void XAbstractThread::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XAbstractThread::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

void XAbstractThread::yield() noexcept
{ taskYIELD();}

bool XAbstractThread::isRunningInThread() noexcept
{ return !static_cast<bool>(xPortIsInsideInterrupt()); }

std::size_t XAbstractThread::thread_count() noexcept
{ return m_th_cnt_.loadAcquire(); }

void * XAbstractThread::thread_handle() const noexcept
{ return m_thread_.loadAcquire(); }

int XAbstractThread::thread_id() const noexcept
{ return m_id_.loadAcquire(); }

#define TH auto const th { thread_handle() }

void XAbstractThread::start() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskResume(static_cast<TaskHandle_t>(th)); }

void XAbstractThread::stop() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskSuspend(static_cast<TaskHandle_t>(th)); }

void XAbstractThread::setPriority(uint32_t const p) const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskPrioritySet(static_cast<TaskHandle_t>(thread_handle()),p); }

void XAbstractThread::destroy() noexcept {
    TH;CHECK_EMPTY(th,return);
    vTaskSuspend(static_cast<TaskHandle_t>(th));
    vTaskDelete(static_cast<TaskHandle_t>(th));
    m_thread_.storeRelease({});
}

XAbstractThread::~XAbstractThread()
{ destroy(); }

void XAbstractThread::setInfo(void * const th) noexcept{
    if (th) {
        auto const th_{ static_cast<TaskHandle_t>(th) };
        m_thread_.storeRelease(th_);
        auto const id { m_th_cnt_.fetchAndAddOrdered(1) };
        m_id_.storeRelease(static_cast<int>(id));
        vTaskSuspend(th_);
    }
}

void XAbstractThread::swap(XAbstractThread & o) noexcept {
    TaskCriticalArea c;
    auto const id { m_id_.loadAcquire() };
    auto const th{ m_thread_.loadAcquire() };
    m_id_.storeRelease(o.m_id_.loadAcquire());
    m_thread_.storeRelease(o.m_thread_.loadAcquire());
    o.m_id_.storeRelease(id);
    o.m_thread_.storeRelease(th);
}

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
