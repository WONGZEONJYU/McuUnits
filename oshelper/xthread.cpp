#include <xthread.hpp>
#include <criticalarea.hpp>
#if defined(FREERTOS) || defined(USE_FREERTOS)
#include <task.h>

void XThreadBase::taskReturn() noexcept
{ vTaskDelete(nullptr); }

void XThreadBase::sleep_for(std::size_t const ms) noexcept
{ vTaskDelay(ms); }

void XThreadBase::sleep_until(std::size_t & pre,std::size_t const ms) noexcept
{ vTaskDelayUntil(reinterpret_cast<TickType_t*>(&pre),ms); }

std::size_t XThreadBase::thread_count() noexcept
{ return m_th_cnt_.load(std::memory_order_relaxed); }

void * XThreadBase::thread_handle() const noexcept
{ return m_thread_.load(std::memory_order_acquire); }

int XThreadBase::thread_id() const noexcept
{ return m_id_.load(std::memory_order_relaxed); }

void XThreadBase::start() const noexcept {
    if (m_thread_.load(std::memory_order_relaxed)) {
        vTaskResume(static_cast<TaskHandle_t>(m_thread_.load(std::memory_order_relaxed)));
    }
}

void XThreadBase::stop() const noexcept {
    if (m_thread_.load(std::memory_order_relaxed)) {
        vTaskSuspend(static_cast<TaskHandle_t>(m_thread_.load(std::memory_order_relaxed)));
    }
}

void XThreadBase::setPriority(uint32_t const p) const noexcept {
    if (m_thread_.load(std::memory_order_relaxed)) {
        vTaskPrioritySet(static_cast<TaskHandle_t>(m_thread_.load(std::memory_order_relaxed)),p);
    }
}

void XThreadBase::destroy() noexcept {
    if (m_thread_.load(std::memory_order_relaxed)) {
        vTaskSuspend(static_cast<TaskHandle_t>(m_thread_.load(std::memory_order_relaxed)));
        vTaskDelete(static_cast<TaskHandle_t>(m_thread_.load(std::memory_order_relaxed)));
        m_thread_.store({},std::memory_order_relaxed);
    }
}

XThreadBase::~XThreadBase()
{ destroy(); }

void XThreadBase::setInfo(void * const th) {
    if (th) {
        auto const th_ { static_cast<tskTaskControlBlock*>(th) };
        m_thread_.store(th_,std::memory_order_relaxed);
        m_id_ = static_cast<int>(m_th_cnt_.fetch_add(1,std::memory_order_relaxed));
        vTaskSuspend(th_);
    }
}

void XThreadBase::swap(XThreadBase & o) noexcept {
    m_id_.store(o.m_id_.exchange(m_id_.load(std::memory_order_relaxed),std::memory_order_relaxed),std::memory_order_relaxed);
    m_thread_.store(o.m_thread_.exchange(m_thread_.load(std::memory_order_relaxed),std::memory_order_relaxed),std::memory_order_relaxed);
}

void XThreadBase::createTask(void(*f)(void*)
                                , std::size_t const uxStackDepth
                                , void * const pvParameters
                                , void * const puxStackBuffer
                                , void * const pxTaskBuffer) noexcept
{
    XStringStream ss {};
    ss << GET_STR(thread:) << m_th_cnt_.load(std::memory_order_relaxed);
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

XThreadDynamic:: XThreadDynamic(XThreadDynamic && o) noexcept
{ XThreadBase::swap(o);}

XThreadDynamic & XThreadDynamic::operator=(XThreadDynamic && o) noexcept
{ XThreadDynamic{std::move(o)}.swap(*this); return *this; }

#endif

#endif
