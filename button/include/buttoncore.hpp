#ifndef BUTTON_CORE_HPP
#define BUTTON_CORE_HPP 1

#include <wglobal.hpp>
#include <xmemory.hpp>

class ButtonCore;
class ButtonCorePrivate;
class Button;

class ButtonCoreData {
protected:
    ButtonCoreData() = default;
public:
    virtual ~ButtonCoreData() = default;
    ButtonCore * q_ptr{};
};

class ButtonCore final : XSingleton<ButtonCore> {
    W_DISABLE_COPY_MOVE(ButtonCore)
    W_DECLARE_PRIVATE_D(m_d_ptr_,ButtonCore)
    X_TWO_PHASE_CONSTRUCTION_CLASS
    XUniquePtr<ButtonCoreData> m_d_ptr_{};

public:
    static ButtonCore * handle() noexcept;
    void exec() noexcept;
    void push(XSharedPtr<Button> const &) noexcept;
    void remove(void * ) noexcept;
private:
    ButtonCore() = default;
    ~ButtonCore() = default;
    bool construct_() noexcept;
};

#endif
