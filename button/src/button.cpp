#include <button.hpp>
#include "button_p.hpp"
#include <buttoncore.hpp>
#include <xhelper.hpp>

void ButtonPrivate::init_(uint16_t const doubleClickTime,
                   uint16_t const longClickTime,
                   uint16_t const debounceTime,
                   uint16_t const doubleClickNum ,
                   bool const level,void * const d) {
    m_doubleClickTime_ = doubleClickTime;
    m_longClickTime_ = longClickTime;
    m_debounceTime_ = debounceTime;
    m_doubleClickNum_ = doubleClickNum;
    m_level_ = level;
    m_data_ = d;
}

void ButtonPrivate::exec_() {
    if (!m_getPin_){ return; }
    if (m_keyStep_ > 0U || m_keyRepeat_ > 0U) { ++m_keyTicks_; }

    switch (m_keyStep_) {
        case 0: {
            if ( m_getPin_() == m_level_) {
                if (++m_debounce_ >= m_debounceTime_ ) {
                    m_debounce_ = {};
                    if ( m_getPin_() == m_level_ ){ ++m_keyStep_; }
                }
            } else { m_debounce_ = {}; }
            break;
        }
        case 1:{
            if ( m_getPin_() == m_level_ && !m_reuse_ && m_keyTicks_ >= m_longClickTime_ ) { /*超时执行长按动作--动作时长2秒*/
                m_reuse_ = true;
                q_func()->longClickedSignal(ButtonMode::LongClicked,m_data_);
            }else if (m_getPin_() != m_level_){
                if (m_reuse_) { /*已经执行长按动作*/
                    m_keyTicks_ = {};
                    m_keyRepeat_ = {};
                    m_keyStep_ = {}; /*回到 0状态 判断按键*/
                    m_reuse_ = {};
                    q_func()->longReleaseSignal(ButtonMode::LongRelease,m_data_);
                }else { /*未执行长按动作*/
                    m_keyStep_ = {};
                    if (!m_keyRepeat_){ m_lastTime_ = m_keyTicks_; }
                    ++m_keyRepeat_;
                }
            }else if( m_getPin_() == m_level_ && m_reuse_ ){
                q_func()->longPressHoldSignal(ButtonMode::LongPressHold,m_data_);
            }else {}
            break;
        }
        default:
            break;
    }

    constexpr auto f { [](uint16_t const OldTime,uint16_t const NewTime ) noexcept -> uint16_t {
        return NewTime < OldTime ? NewTime + static_cast<uint16_t>(0xFFFF) - OldTime : NewTime - OldTime;
    }};

    auto const clearValue { [this]()noexcept{
        m_keyRepeat_ = {};
        m_keyTicks_ = {};
        m_lastTime_ = {};
    } };

    if (auto const calcResult{ f(m_lastTime_, m_keyTicks_) }
        ; calcResult > m_doubleClickTime_ && m_keyRepeat_ > 0 )
    {
        /* 短按处理*/
        clearValue();
        q_func()->shortClickedSignal(ButtonMode::ShortClicked,m_data_);
    }else if ( calcResult < m_doubleClickTime_ && m_keyRepeat_ == m_doubleClickNum_ ){
        /*双击处理*/
        clearValue();
        q_func()->doubleClickedSignal(ButtonMode::DoubleClicked,m_data_);
    } else{
    }
}

XSharedPtr<Button> Button::create(levelDetectFunc && f,bool const level
    ,void * const userdata,ButtonTime const & bv) noexcept
{
    auto const core { ButtonCore::handle() };
    CHECK_EMPTY(core,return {});
    auto ret{ CreateSharedPtr({},Parameter { std::move(f),level,userdata,bv } ) };
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
