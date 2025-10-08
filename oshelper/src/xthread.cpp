#include <xthread.hpp>
#include <criticalarea.hpp>
#include <xhelper.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

void XThreadAbstract::taskReturn() noexcept
{ vTaskDelete(nullptr); }

void XThreadAbstract::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XThreadAbstract::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

void XThreadAbstract::yield() noexcept
{ taskYIELD();}

bool XThreadAbstract::isRunningInThread() noexcept
{ return !static_cast<bool>(xPortIsInsideInterrupt()); }

std::size_t XThreadAbstract::thread_count() noexcept
{ return m_th_cnt_.loadAcquire(); }

void * XThreadAbstract::thread_handle() const noexcept
{ return m_thread_.loadAcquire(); }

int XThreadAbstract::thread_id() const noexcept
{ return m_id_.loadAcquire(); }

#define TH auto const th { thread_handle() }

void XThreadAbstract::start() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskResume(static_cast<TaskHandle_t>(th)); }

void XThreadAbstract::stop() const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskSuspend(static_cast<TaskHandle_t>(th)); }

void XThreadAbstract::setPriority(uint32_t const p) const noexcept
{ TH;CHECK_EMPTY(th,return); vTaskPrioritySet(static_cast<TaskHandle_t>(thread_handle()),p); }

void XThreadAbstract::destroy() noexcept {
    TH;CHECK_EMPTY(th,return);
    vTaskSuspend(static_cast<TaskHandle_t>(th));
    vTaskDelete(static_cast<TaskHandle_t>(th));
    m_thread_.storeRelease({});
}

XThreadAbstract::~XThreadAbstract()
{ destroy(); }

void XThreadAbstract::setInfo(void * const th) noexcept{
    if (th) {
        auto const th_{ static_cast<TaskHandle_t>(th) };
        m_thread_.storeRelease(th_);
        auto const id { m_th_cnt_.fetchAndAddOrdered(1) };
        m_id_.storeRelease(static_cast<int>(id));
        vTaskSuspend(th_);
    }
}

void XThreadAbstract::swap(XThreadAbstract & o) noexcept {
    TaskCriticalArea c;
    auto const id { m_id_.loadAcquire() };
    auto const th{ m_thread_.loadAcquire() };
    m_id_.storeRelease(o.m_id_.loadAcquire());
    m_thread_.storeRelease(o.m_thread_.loadAcquire());
    o.m_id_.storeRelease(id);
    o.m_thread_.storeRelease(th);
}

void XThreadAbstract::createTask(void(*f)(void*)
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
{ XThreadAbstract::swap(o);}

XThreadDynamic & XThreadDynamic::operator=(XThreadDynamic && o) noexcept
{ XThreadDynamic{std::move(o)}.swap(*this); return *this; }

#endif

#endif
