#include <button.hpp>
#include "button_p.hpp"
#include <buttoncore.hpp>
#include <xhelper.hpp>

XSharedPtr<Button> Button::create(levelDetectFunc && f,bool const level
    ,void * const userdata,ButtonTime const & bv) noexcept
{
    auto const core { ButtonCore::handle() };
    CHECK_EMPTY(core,return {});
    auto const ret{ CreateSharedPtr({},Parameter { std::move(f),level,userdata,bv } ) };
    CHECK_EMPTY(ret,return {});
    core->push(ret);
    return ret;
}

Button::Button() = default;

Button::~Button() = default;

void Button::shortClickedSignal(ButtonMode const type,void * const d)
{ Emit::emit_(this,&Button::shortClickedSignal,nullptr,type,d); }

void Button::doubleClickedSignal(ButtonMode const type,void * const d)
{ Emit::emit_(this,&Button::doubleClickedSignal,nullptr,type,d); }

void Button::longClickedSignal(ButtonMode const type,void * const d)
{ Emit::emit_(this,&Button::longClickedSignal,nullptr,type,d); }

void Button::longPressHoldSignal(ButtonMode const type,void * const d)
{ Emit::emit_(this,&Button::longPressHoldSignal,nullptr,type,d); }

void Button::longReleaseSignal(ButtonMode const type,void * const d)
{ Emit::emit_(this,&Button::longReleaseSignal,nullptr,type,d); }

void Button::exec_() noexcept
{ W_D(Button); d->exec_(); }

bool Button::construct_() noexcept {
    auto dd{ makeUnique<ButtonPrivate>(this) };
    CHECK_EMPTY(dd,return {});
    m_d_ptr_ = std::move(dd);
    return true;
}

bool Button::construct_(levelDetectFunc && f, bool const level,
    void * const userdata, ButtonTime const & bv) noexcept
{
    CHECK_EMPTY(construct_(),return {});
    W_D(Button);
    std::ranges::swap(d->m_getPin_,f);
    d->init_(bv.m_doubleClickTime,bv.m_longClickTime
        ,bv.m_debounceTime,bv.m_doubleClickNum,level,userdata);
    return true;
}
