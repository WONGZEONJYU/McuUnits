#include <button.hpp>

Button::~Button() noexcept
{ Remove(); }

void Button::input(const uint16_t &doubleClickTime,
                   const uint16_t &longClickTime,
                   const uint16_t &debunceTime,
                   const uint16_t &doubleClickNum ,
                   const bool &level,void *d) {
    m_time_.m_double_Click_Time = doubleClickTime;
    m_time_.m_long_Click_Time = longClickTime;
    m_time_.m_debunce_Time = debunceTime;
    m_time_.m_double_Click_Num = doubleClickNum;
    m_time_.level = level;
    m_bd_.data = d;
}

void Button::ShortClicked_signal(const int & type,void *d)
{ _Sig_::_emit_(this,&Button::ShortClicked_signal,nullptr,type,d); }

void Button::DoubleClicked_signal(const int & type,void *d)
{ _Sig_::_emit_(this,&Button::DoubleClicked_signal,nullptr,type,d); }

void Button::LongClicked_signal(const int & type,void *d)
{ _Sig_::_emit_(this,&Button::LongClicked_signal,nullptr,type,d); }

void Button::LongPressHold_signal(const int & type,void *d)
{ _Sig_:: _emit_(this,&Button::LongPressHold_signal,nullptr,type,d); }

void Button::LongRelease_signal(const int & type,void *d)
{ _Sig_:: _emit_(this,&Button::LongRelease_signal,nullptr,type,d); }

void Button::exec() {
    for (const auto &item : m_button_list){
        item->exec_();
    }
}

void Button::exec_() {
    if (!m_get_pin_){
        return;
    }

    if ((m_bd_.KEY_STEP > 0U) || (m_bd_.KeyRepeat > 0U)){
        m_bd_.KeyTicks++;
    }

    switch (m_bd_.KEY_STEP){
        case 0:{
            if ( m_get_pin_() == m_time_.level){

                if (++m_bd_.Debounce >= m_time_.m_debunce_Time){

                    m_bd_.Debounce = 0U;

                    if ( m_get_pin_() == m_time_.level ){

                        m_bd_.KEY_STEP++;
                    }
                }
            }
            else{
                m_bd_.Debounce = 0U;
            }
            break;
        }

        case 1:{
            if ( (m_get_pin_() == m_time_.level) && (!m_bd_.ReuseValue) &&
                 (m_bd_.KeyTicks >= m_time_.m_long_Click_Time) ) { /*超时执行长按动作--动作时长2秒*/

                m_bd_.ReuseValue = true;
                LongClicked_signal(LongClicked,m_bd_.data);

            }else if (m_get_pin_() != m_time_.level){
                if (m_bd_.ReuseValue) { /*已经执行长按动作*/

                    m_bd_.KeyTicks = {};
                    m_bd_.KeyRepeat = {};
                    m_bd_.KEY_STEP = {}; /*回到 0状态 判断按键*/
                    m_bd_.ReuseValue = {};
                    LongRelease_signal(LongRelease,m_bd_.data);
                }else { /*未执行长按动作*/

                    m_bd_.KEY_STEP = {};
                    if (!m_bd_.KeyRepeat){
                        m_bd_.LastTime = m_bd_.KeyTicks;
                    }
                    m_bd_.KeyRepeat++;
                }
            }else if( (m_get_pin_() == m_time_.level) && m_bd_.ReuseValue ){
                LongPressHold_signal(LongPressHold,m_bd_.data);
            }
            break;
        }
        default:
            break;
    }

    auto f{[=](const ulBase_Type &OldTime,
              const ulBase_Type &NewTime )mutable ->ulBase_Type {
        if (NewTime < OldTime) {
            return ((NewTime + 0xFFFFU) - OldTime);
        }else{
            return (NewTime - OldTime);
        }
    }};

    if ( (f(m_bd_.LastTime, m_bd_.KeyTicks) > m_time_.m_double_Click_Time) &&
         (m_bd_.KeyRepeat > 0)){
        /* 短按处理*/
        m_bd_.KeyRepeat = {};
        m_bd_.KeyTicks = {};
        m_bd_.LastTime = {};
        ShortClicked_signal(ShortClicked,m_bd_.data);
    }else if ( (f(m_bd_.LastTime, m_bd_.KeyTicks) < m_time_.m_double_Click_Time) &&
              (m_bd_.KeyRepeat == m_time_.m_double_Click_Num) ){
        /*双击处理*/
        m_bd_.KeyRepeat = {};
        m_bd_.KeyTicks = {};
        m_bd_.LastTime = {};
        DoubleClicked_signal(DoubleClicked,m_bd_.data);
    } else{
    }
}

void Button::Add()
{ m_button_list.emplace_back(this); }

void Button::Remove() {
    std::erase_if(m_button_list,[this](const auto p){
        return p == this;
    });
}
