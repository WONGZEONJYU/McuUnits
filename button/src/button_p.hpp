#ifndef BUTTON_P_HPP
#define BUTTON_P_HPP

#include <button.hpp>

class ButtonPrivate final : public ButtonData {
    W_DECLARE_PUBLIC(Button)
    void * m_data_{};
    std::function<bool()> m_getPin_{};

    std::uint16_t m_lastTime_{}  /*上一次双击时间记录*/
            ,m_keyTicks_{}  /*按键计时标志*/
            ,m_keyStep_{}   /*按键状态*/
            ,m_keyRepeat_{}  /*双击计次*/
            ,m_debounce_{}   /*消抖计数*/
            ,m_doubleClickTime_{}
            ,m_longClickTime_{}
            ,m_debounceTime_{}
            ,m_doubleClickNum_{}
            ,m_reuse_ : 1{} /*复用变量*/
            ,m_level_ : 1 {}; /*电平方向*/

#if 0
    struct DB {
        void * data{};
        std::size_t LastTime{},  /*上一次双击时间记录*/
                KeyTicks{},  /*按键计时标志*/
                KEY_STEP{},   /*按键状态*/
                KeyRepeat{},  /*双击计次*/
                Debounce{};   /*消抖计数*/
        uint32_t ReuseValue : 1{}; /*复用变量*/
    }m_bd_{};
    struct TIME {
        std::size_t m_doubleClickTime{},m_longClickTime{},
            m_debounceTime{},m_doubleClickNum{};
        uint32_t level : 1{};
    }m_time_ {};
#endif

public:
    explicit ButtonPrivate(Button * const o)
    { q_ptr = o; }

    ~ButtonPrivate() override = default;

    void init_(uint16_t doubleClickTime,
                uint16_t longClickTime,
                uint16_t debounceTime,
                uint16_t doubleClickNum ,
                bool level,void * d);
    void exec_();
};

inline void ButtonPrivate::init_(uint16_t const doubleClickTime,
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

inline void ButtonPrivate::exec_() {
    if (!m_getPin_){ return; }
    if (m_keyStep_ > 0U || m_keyRepeat_  > 0U) { ++m_keyTicks_; }

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
                q_func()->longClickedSignal(Mode::LongClicked,m_data_);
            }else if (m_getPin_() != m_level_){
                if (m_reuse_) { /*已经执行长按动作*/
                    m_keyTicks_ = {};
                    m_keyRepeat_ = {};
                    m_keyStep_ = {}; /*回到 0状态 判断按键*/
                    m_reuse_ = {};
                    q_func()->longReleaseSignal(Mode::LongRelease,m_data_);
                }else { /*未执行长按动作*/
                    m_keyStep_ = {};
                    if (!m_keyRepeat_){ m_lastTime_ = m_keyTicks_; }
                    ++m_keyRepeat_;
                }
            }else if( m_getPin_() == m_level_ && m_reuse_ ){
                q_func()->longPressHoldSignal(Mode::LongPressHold,m_data_);
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
        q_func()->shortClickedSignal(Mode::ShortClicked,m_data_);
    }else if ( calcResult < m_doubleClickTime_ && m_keyRepeat_ == m_doubleClickNum_ ){
        /*双击处理*/
        clearValue();
        q_func()->doubleClickedSignal(Mode::DoubleClicked,m_data_);
    } else{
    }
}

#endif
