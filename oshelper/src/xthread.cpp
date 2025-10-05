#include <xthread.hpp>
#include <criticalarea.hpp>
#include <xhelper.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

void XThreadBase::taskReturn() noexcept
{ vTaskDelete(nullptr); }

void XThreadBase::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XThreadBase::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

void XThreadBase::yield() noexcept
{ taskYIELD();}

bool XThreadBase::isRunningInThread() noexcept
{ return !static_cast<bool>(xPortIsInsideInterrupt()); }

std::size_t XThreadBase::thread_count() noexcept
{ return m_th_cnt_.loadAcquire(); }

void * XThreadBase::thread_handle() const noexcept
{ return m_thread_.loadAcquire(); }

int XThreadBase::thread_id() const noexcept
{ return m_id_.loadAcquire(); }

#define TH auto const th { thread_handle() }

void XThreadBase::start() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskResume(static_cast<TaskHandle_t>(th)); }

void XThreadBase::stop() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskSuspend(static_cast<TaskHandle_t>(th)); }

void XThreadBase::setPriority(uint32_t const p) const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskPrioritySet(static_cast<TaskHandle_t>(thread_handle()),p); }

void XThreadBase::destroy() noexcept {
    TH;CHECK_EMPTY(th,return);
    vTaskSuspend(static_cast<TaskHandle_t>(th));
    vTaskDelete(static_cast<TaskHandle_t>(th));
    m_thread_.storeRelease({});
}

XThreadBase::~XThreadBase()
{ destroy(); }

void XThreadBase::setInfo(void * const th) noexcept{
    if (th) {
        auto const th_{ static_cast<TaskHandle_t>(th) };
        m_thread_.storeRelease(th_);
        auto const id { m_th_cnt_.fetchAndAddOrdered(1) };
        m_id_.storeRelease(static_cast<int>(id));
        vTaskSuspend(th_);
    }
}

void XThreadBase::swap(XThreadBase & o) noexcept {
    TaskCriticalArea c;
    auto const id { m_id_.loadAcquire() };
    auto const th{ m_thread_.loadAcquire() };
    m_id_.storeRelease(o.m_id_.loadAcquire());
    m_thread_.storeRelease(o.m_thread_.loadAcquire());
    o.m_id_.storeRelease(id);
    o.m_thread_.storeRelease(th);
}

void XThreadBase::createTask(void(*f)(void*)
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

#if configSUPPORT_DYNAMIC_ALLOCATION > 0

XThreadDynamic::XThreadDynamic(XThreadDynamic && o) noexcept
{ XThreadBase::swap(o);}

XThreadDynamic & XThreadDynamic::operator=(XThreadDynamic && o) noexcept
{ XThreadDynamic{std::move(o)}.swap(*this); return *this; }

#endif

#endif
