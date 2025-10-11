#ifndef BUTTON_P_HPP
#define BUTTON_P_HPP 1

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

public:
    constexpr explicit ButtonPrivate(Button * const o)
    { q_ptr = o; }

    ~ButtonPrivate() override = default;

    void init_(uint16_t doubleClickTime,
                uint16_t longClickTime,
                uint16_t debounceTime,
                uint16_t doubleClickNum ,
                bool level,void * d);
    void exec_();
};

#endif
