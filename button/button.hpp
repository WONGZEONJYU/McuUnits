#ifndef BUTTON_H_
#define BUTTON_H_

#include <wobject.hpp>
#include <functional>
#include <list>

//#define DoubleClickTime 200     /*双击时间(200ms)，单位时间内单击的次数*/
//#define LongClickTime   600     /*长按时间(600ms)，大于单位时间识别为长按*/
//#define DebunceTime     10      /*消抖时间(10ms)，大于单位时间消抖*/
//#define DoubleClickNum  2       /*连按次数(2c)，单位时间内需按下的次数，2为双击*/

class Button final :public WObject {
    W_DISABLE_COPY_MOVE(Button)

    inline static std::list<Button *> m_button_list {};

    struct DB{
        uint16_t LastTime{},  /*上一次双击时间记录*/
                KeyTicks{};  /*按键计时标志*/
        uint8_t KEY_STEP{},   /*按键状态*/
                KeyRepeat{},  /*双击计次*/
                Debounce{};   /*消抖计数*/
        bool ReuseValue{}; /*复用变量*/
        void *data{};
    }m_bd_{};

    struct TIME{
        uint16_t m_double_Click_Time{},m_long_Click_Time{},
        m_debunce_Time{},m_double_Click_Num{};
        bool level{};
    }m_time_ __ALIGNED(4){};

    std::function<bool()> m_get_pin_{};

public:
    enum{
        ShortClicked = 1,
        DoubleClicked = 2,
        LongClicked = 3,
        LongPressHold = 4,
        LongRelease = 5,
    };

    enum reference{
        DoubleClickTime = 200,
        LongClickTime = 600,
        DebunceTime = 10,
        DoubleClickNum = 2,
    };

    explicit Button() = default;

    template<typename FUNC>
    explicit Button(FUNC &&f,const bool &level = {},void* d = {},
                    const uint16_t &doubleClickTime = DoubleClickTime,
                    const uint16_t &longClickTime = LongClickTime,
                    const uint16_t &debunceTime = DebunceTime,
                    const uint16_t &doubleClickNum = DoubleClickNum) noexcept {
        init(f,level,d,doubleClickTime,longClickTime,debunceTime,doubleClickNum);
    }

    template<typename OBJ,typename FUNC>
    explicit Button(OBJ * const o,FUNC &&f,const bool &level = {},void* d = {},
                    const uint16_t &doubleClickTime = DoubleClickTime,
                    const uint16_t &longClickTime = LongClickTime,
                    const uint16_t &debunceTime = DebunceTime,
                    const uint16_t &doubleClickNum = DoubleClickNum) noexcept {
        init(o,f,level,d,doubleClickTime,longClickTime,debunceTime,doubleClickNum);
    }

    template<typename OBJ,typename FUNC>
    void init(OBJ * const o,FUNC &&f,const bool level = false,void* d = nullptr,
              const usBase_Type doubleClickTime = DoubleClickTime,
              const usBase_Type longClickTime = LongClickTime,
              const usBase_Type debunceTime = DebunceTime,
              const usBase_Type doubleClickNum = DoubleClickNum) noexcept
    {
        if (!m_get_pin_){
            input(doubleClickTime,longClickTime,debunceTime,
                  doubleClickNum,level,d);
            m_get_pin_ = std::bind(std::forward<FUNC>(f),o);
            Add();
        }
    }

    template<typename FUNC>
    void init(FUNC &&f,const bool level = false,void* d = nullptr,
              const usBase_Type doubleClickTime = DoubleClickTime,
              const usBase_Type longClickTime = LongClickTime,
              const usBase_Type debunceTime = DebunceTime,
              const usBase_Type doubleClickNum = DoubleClickNum)noexcept
    {
        if (!m_get_pin_){
            input(doubleClickTime,longClickTime,debunceTime,
                  doubleClickNum,level,d);
            m_get_pin_ = std::bind(std::forward<FUNC>(f));
            Add();
        }
    }

    ~Button() noexcept override;

    static void exec();

signals:
    void ShortClicked_signal(const int &,void *);
    void DoubleClicked_signal(const int &,void *);
    void LongClicked_signal(const int &,void *);
    void LongPressHold_signal(const int &,void *);
    void LongRelease_signal(const int &,void *);

private:
    void exec_();
    void input(const uint16_t &doubleClickTime,
               const uint16_t &longClickTime,
               const uint16_t &debunceTime,
               const uint16_t &doubleClickNum ,
               const bool &level,void *d);
    void Add();
    void Remove();
};

#endif
