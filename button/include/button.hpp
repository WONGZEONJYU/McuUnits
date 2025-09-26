#ifndef BUTTON_HPP
#define BUTTON_HPP 1

#include <wobject.hpp>
#include <functional>
#include <xcontainer.hpp>

#if 0
    #define DoubleClickTime 200     /*双击时间(200ms)，单位时间内单击的次数*/
    #define LongClickTime   600     /*长按时间(600ms)，大于单位时间识别为长按*/
    #define DebunceTime     10      /*消抖时间(10ms)，大于单位时间消抖*/
    #define DoubleClickNum  2       /*连按次数(2c)，单位时间内需按下的次数，2为双击*/
#endif

enum class ButtonMode : int {
    ShortClicked = 1,
    DoubleClicked ,
    LongClicked ,
    LongPressHold ,
    LongRelease ,
};

enum ButtonTimeValue : uint16_t {
    DoubleClickTime = 200,
    LongClickTime = 600,
    DebounceTime = 10,
    DoubleClickNum = 2,
};

struct ButtonTime final {

    uint16_t m_doubleClickTime {DoubleClickTime},
        m_longClickTime {LongClickTime},
        m_debounceTime {DebounceTime},
        m_doubleClickNum {DoubleClickNum};

    constexpr ButtonTime(uint16_t const doubleClickTime = DoubleClickTime
        ,uint16_t const longClickTime = LongClickTime
        ,uint16_t const debounceTime = DebounceTime
        ,uint16_t const doubleClickNum = DoubleClickNum):
        m_doubleClickTime(doubleClickTime),m_longClickTime(longClickTime),m_debounceTime(debounceTime)
        ,m_doubleClickNum(doubleClickNum){}
};

class Button;
class ButtonPrivate;
using levelDetectFunc = std::function<bool()>;

class ButtonData {
protected:
    ButtonData() = default;
public:
    virtual ~ButtonData() = default;
    Button * q_ptr{};
};

class Button final : public WObject ,XTwoPhaseConstruction<Button> {
    W_DISABLE_COPY_MOVE(Button)
    W_DECLARE_PRIVATE_D(m_d_ptr_,Button)
    X_TWO_PHASE_CONSTRUCTION_CLASS
    XUniquePtr<ButtonData> m_d_ptr_{};

public:
    static XSharedPtr<Button> create(levelDetectFunc && f,bool = {} ,void * = {} ,ButtonTime const & = {}) noexcept;
    ~Button() override;

#define FOR_EACH_MAKE(f) \
    f(shortClicked) f(doubleClicked) f(longClicked) \
    f(longPressHold) f(longRelease)

#define MAKE_CALL_ON(name) \
    template<typename ... Args> \
    bool callOn##name(Args && ...args) const noexcept \
    { return connect(this,&Button::name##Signal,std::forward<Args>(args)...); }

    FOR_EACH_MAKE(MAKE_CALL_ON)

#undef MAKE_CALL_ON
#undef FOR_EACH_MAKE

W_SIGNALS:
    void shortClickedSignal(ButtonMode,void *);
    void doubleClickedSignal(ButtonMode,void *);
    void longClickedSignal(ButtonMode,void *);
    void longPressHoldSignal(ButtonMode,void *);
    void longReleaseSignal(ButtonMode,void *);

private:
    explicit Button();
    void exec_() noexcept;
    bool construct_() noexcept;
    bool construct_(levelDetectFunc && ,bool ,void * ,ButtonTime const &) noexcept;
    friend class ButtonCore;
    friend class ButtonCorePrivate;
};

#endif
