#ifndef BUTTON_CORE_P_HPP
#define BUTTON_CORE_P_HPP 1

#include <buttoncore.hpp>
#include <xcontainer.hpp>
#include <button.hpp>
#include <ranges>
#include <xhelper.hpp>

class ButtonCorePrivate final: public ButtonCoreData {
    W_DECLARE_PUBLIC(ButtonCore)
    XMap<void *,XSharedPtr<Button>> m_buttons{};

public:
    constexpr explicit ButtonCorePrivate(ButtonCore * const o)
    { q_ptr = o; }
    ~ButtonCorePrivate() override = default;
    void addButton(XSharedPtr<Button> const &) noexcept;
    void removeButton(void *) noexcept;
    void exec() const noexcept;
};

#endif
